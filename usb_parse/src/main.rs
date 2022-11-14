//! Executable to parse struct-formatted sensor data coming from Teensy into a JSON-compatible format.
//! 
//! File: main.rs
//! Author: Derek Guo
//! Version: 1
//! Date: 2022-11-13
//! 
//! Copyright (c) 2022

/* Namespaces */
pub mod structs;

use {
    crate::{
        structs::{
            TeensyCanData,
            SensorVals,
        },
    },
    std::{
        fmt,
        error,
        io::{
            self,
            stdout,
            Read,
            Write,
        },
        sync::{
            atomic::{
                AtomicBool,
                Ordering
            },
            Arc,
        },
        path::Path,
        fs::File,
    },
    // libusb as usb,
    anyhow::bail,
    serialport::{
        SerialPort,
        ClearBuffer,
    },
    serde_json::Value,
};

/* Custom error handling */
#[derive(Debug)]
enum TelemetryBaseStationError {
    ConnectionError(serialport::Error),
    DisconnectError,
    ClearDataError,
    ReadError(std::io::Error),
    BufferLenError(usize),
    DeserializeError(bincode::Error),
}

impl fmt::Display for TelemetryBaseStationError {
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        use TelemetryBaseStationError::*;
        match self {
            ConnectionError(e) => write!(f, "Error when opening serial port: {:?}", e),
            DisconnectError => write!(f, "Teensy disconnected"),
            ClearDataError => write!(f, "Unable to clear existing accumulated data"),
            ReadError(e) => write!(f, "Read error: {:?}", e),
            BufferLenError(l) => write!(f, "Mismatched buffer length: {}", l),
            DeserializeError(e) => write!(f, "Error in initial parse: {:?}", e),
        }
    }
}

impl error::Error for TelemetryBaseStationError {}

// Interrupt handler for worst case scenarios
fn sigint_exit() {
    std::process::exit(69);
}

/* Port names */
// Unix and Unix-like, including Linux and MacOS
// Static, dependent on device end
#[cfg(unix)]
const DEFAULT_TTY: &str = "/dev/tty.usbmodem115442301";

// Windows based systems
// Relative and variable, dependent on existing ports in use
#[cfg(windows)]
const DEFAULT_TTY: &str = "COM1"; // TODO: Find common standard

/* Expected buffer length */
const BUFFER_SIZE: usize = 27;

fn main() -> anyhow::Result<()> {
    /* Initializations */
    // let context = usb::Context::new().context("Failed to access USB context")?;

    // Lock stdout for more efficient bodies
    let mut out_lock = stdout().lock();

    /* Handle SIGINT (abort) */
    // Running check
    let running = Arc::new(AtomicBool::new(true));
    let r = running.clone();

    // Initiate the unwinding process (akin to panic);
    // additional checks throughout the program will allow the program to exit proper
    // In some cases, an immediate exit is necessary.
    ctrlc::set_handler(move || {
        if let Err(_) = writeln!(io::stderr().lock(), "\nINT detected, unwinding...") {
            sigint_exit();
        };
        r.store(false, Ordering::Relaxed);
    }).expect("Error setting Ctrl-C handler");

    /* Data storage */
    // Prevent reallocation during loop

    // Data buffer
    // Capacity based on LoRa packet size limit of 252
    let mut sensor_buf: Vec<u8> = vec![0; 252];
    let mut len: usize;

    // Formatted structs
    let mut sensor_struct: TeensyCanData;

    // Formatted objects
    let mut sensor_vals: SensorVals;

    /* Sensor information */
    // This is obtained from a JSON objectin refs/sensor_lists.json
    // Due to its dynamic nature and difficulty maintaining a large struct or array
    // to account for a growing and ever-temperamental list of sensor values,
    // the dynamic object Value must be used, akin to ordinary JSON-keying.
    let sensor_list: Value;

    {
        // Path to file containing JSON object sensor_list
        // Modern Windows systems allow usage of either / or \, no conditions needed
        // This path assumes program is being run on the top level of this directory,
        // at usb_parse/, where the file is located in src/refs/. 
        // The endgoal is that sensor_list.json is shared amongst other Live/Remote programs
        // and should be accessible from a common directory.
        // When the time comes, make sure to modify this path to lead to the correct file!
        let sl_path = Path::new("./src/refs/sensor_list.json");
        
        // Container string to ensure sufficient lifetime for conversion
        let mut contents = String::new();
        
        // Read from file and parse text as JSON
        sensor_list = serde_json::from_str({
            let mut file = File::open(sl_path).expect("Failed to open JSON file");
            file.read_to_string(&mut contents)
                .expect("Failed to read from JSON file");
            contents.as_str()
        }).expect("Failed to parse JSON from sensor vals");
    }

    writeln!(out_lock, "Base Station Parser")?;

    while running.load(Ordering::Relaxed) {
        /* Find Teensy */
        writeln!(out_lock, "Listening...")?;

        // Using libusb directly
        // let teensy: usb::Device = context.devices().context("Failed to list devices")?
        //     .iter().find(|dev| {
        //         let desc = dev.device_descriptor().expect("Failed to fetch device information");
        //         desc.vendor_id() == 0x16c0_u16 && desc.product_id() == 0x0483_u16
        //     }).unwrap();

        // Using serialport
        let mut teensy: Box<dyn SerialPort> = {
            let tty: Box<dyn SerialPort>;

            // Listen (via attempting to open)
            // No timeout, unless interrupted keep waiting for connection
            loop { 
                tty = match serialport::new(DEFAULT_TTY, 9600).open() {
                    Ok(p) => p, // Port has been found
                    Err(ref e) if e.kind == serialport::ErrorKind::NoDevice ||
                                  e.kind == serialport::ErrorKind::Io(io::ErrorKind::NotFound) => {
                        // No device, keep listening
                        if !running.load(Ordering::Relaxed) { sigint_exit(); }
                        continue
                    }, 
                    Err(e) => { // An unexpected error occurred
                        let err = TelemetryBaseStationError::ConnectionError(e);
                        writeln!(out_lock, "{}", err)?;
                        bail!(err);
                    }
                };
                break;
            }
            if !running.load(Ordering::Relaxed) { break; }
            tty
        };

        // #[cfg(unix)]
        // port.set_exclusive(false)
        //     .context("Unable to set serial port exclusive to false")?;

        writeln!(out_lock, "Teensy board found: {}", match teensy.name() {
            Some(name) => name,
            None => String::from("Unknown"),
        })?;

        // Clear pre-existing data output from port
        // Handles the case where Teensy is already connected before starting
        if let Err(e) = teensy.clear(ClearBuffer::Input) {
            match e.kind {
                serialport::ErrorKind::NoDevice | serialport::ErrorKind::Io(io::ErrorKind::NotFound) => {
                    // Teensy likely disconnected, return to search
                    writeln!(out_lock, "{}", TelemetryBaseStationError::DisconnectError)?;
                    if !running.load(Ordering::Relaxed) { break; }
                    continue;
                }
                _ => { // An unexpected error occurred
                    let err = TelemetryBaseStationError::ClearDataError;
                    writeln!(out_lock, "{}", err)?;
                    bail!(err);
                }
            }
        }
        
        // writeln!(out_lock, "Receiving data on {}:", DEFAULT_TTY);
        loop {
            /* Read from buffer */
            len = match teensy.read(sensor_buf.as_mut_slice()) {
                Ok(l) => { // Success, get buffer length 
                    // out_lock.write_all(&sensor_buf[..l]).unwrap();
                    if l != BUFFER_SIZE {
                        writeln!(out_lock, "{}", TelemetryBaseStationError::BufferLenError(l))?;
                        if !running.load(Ordering::Relaxed) { break; }
                        continue;
                    }
                    l
                },
                Err(e) => match e.kind() {
                    io::ErrorKind::TimedOut => { // No read from buffer
                        if !running.load(Ordering::Relaxed) { break; }
                        continue;
                    },
                    io::ErrorKind::BrokenPipe => { // Board disconnected, break reading loop
                        writeln!(out_lock, "{}", TelemetryBaseStationError::DisconnectError)?;
                        break;
                    }
                    _ => { // Another error has occurred
                        let err = TelemetryBaseStationError::ReadError(e);
                        writeln!(out_lock, "{}", err)?;
                        bail!(err);
                    }
                }
            };

            /* Initial parse: deserialization */
            sensor_struct = match bincode::deserialize::<TeensyCanData>(&sensor_buf[..len]) {
                Ok(s) => s,
                Err(e) => { // Parsing error
                    let err = TelemetryBaseStationError::DeserializeError(e);
                    writeln!(out_lock, "{}", err)?;
                    bail!(err);
                }
            };
            // writeln!(out_lock, "{:?}", sensor_struct)?;

            /* Reformat data */
            sensor_vals = SensorVals::new(&sensor_struct, &sensor_list);

            /* Print output */
            writeln!(out_lock, "{:?}", sensor_vals)?;
        }
    }

    // Exit from program
    if !running.load(Ordering::Relaxed) {
        sigint_exit()
    }
    Ok(())
}
