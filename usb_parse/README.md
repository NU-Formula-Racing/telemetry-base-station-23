Executable to read, parse, and output data from a Teensy in a fast and safe manner.
This program is called `usb_parse` (on Unix-based systems) and is located in the top level of the directory.
It is designed to communicate with the corresponding firmware located in the `bs_struct` test directory
for a receiver Teensy (RX).

### Running
Running the program is as simple as typing in `./usb_parse` (Unix) and pressing enter.

The program consists of an indefinite loop that listens for a USB port connection matching that of our
Teensy device. To get data to display, connect the corresponding Teensy board to your computer. 
Once you've done so, after a few seconds you should immediately begin seeing data appear on your screen.

Do not that this program has only been tested on Unix-based systems like MacOS, because I (the author) use one.
USB ports on Windows are managed very differently compared to Mac, and therefore may require more complex operations
to procure. Further testing is needed and has been annotated in the source.

If you want to disconnect your Teensy while running the program, you are free to do so,
and the program will not complain.

Ending the program can be done using the generic interrupt signal (Ctrl+C), which will display exit code 69.

### Building
Should you wish to build/rebuild the program yourself, two scripts are provided for you
to automate that process. One is called `run.sh`, which actually builds and runs the program,
and the other is called `clean.sh`, which cleans up compiler lint data? Did I mention lint?

To use these scripts, on Unix-based systems simply run `bash run.sh` or `bash clean.sh` as
necessary. If you're on Windows, pasting the contents of each script and running that works as well.

If you want to *only* build, e.g. to confirm that a change works, just run `cargo build`. Rust's
Cargo manages all dependencies through its `Cargo.toml` file, and due to the complexities of linking
in Rust's more elementary `rustc` compiler, it is highly recommended to just use Cargo commands.
For the same reason, please do not change the structure of this project directory.

If you find that building is taking an especially long time, especially when it comes to the
'serde' and 'serde_derive' dependencies, that is normal and expected due to the guarantees of Rust.
Alternatively,

### Linking to the USB Library
Due to the breadth of the dependencies, it may be necessary to link external, non-Rust libraries,
in particular the system USB library `libusb` as well as a package configuration manager `pkg-config`.
This was necessary when using the Rust dependency `libusb-sys`, but this is currently deprecated
and may not be necessary. Nevertheless, should a linker error arise, installation commands are given
below.

On Mac OS, those commands are

`brew install pkg-config`

`brew insteall libusb`

On Linux, (not tested) `libusb` should be included by default, but if not, its installation command, along with
the one for `pkg-config`, are included as follows.

`sudo apt-get install -y pkg-config`

`sudo apt-get insteall libusb-1.0`

Additional commands for Windows will be added if we get to testing them.

### Files of interest
Brief explanations of the program source, located in `src/`, will be given below.

- `main.rs`: Contains the main pipeline of the executable. Its setup allocates some data for the loop,
    initializes the interrupt handler, and parses the sensor reference list used for formatting.
    Afterwards, the program enters a loop in which it interacts with the Teensy's USB port:
    
    1) The program scans the port lists for one matching that of a Teensy.
    
    2) If the Teensy is found, it connects, opens it, and clears its buffer of outstanding data.

    3) Inside this loop, the program enters and internal loop in which it continuously reads the buffer.

    4) If the buffer has new data, the program reads it. If not, it waits until new data arrives.

    5) The buffered data, initially stored in an array (`Vec`) is deserialized into a holding struct,
       matching that of the original sent data.
    
    6) The data is reformatted into its true values, as some of the deserialized data is still in raw form.

    7) The data is then output to the terminal. This can be expanded on in the future.

    8) Once all is said and done, return to step (3).

    Should the Teensy be disconnected, the program will return to step (1) and listen again.

- `structs.rs`: Contains the design for each of the two structs used in the program.
    The first struct, `TeensyCanData`, represents the minimally-processed data sent
    from the Teensy. This intermediary is necessary to streamline the deserialization
    process, as this data is packed to minimize buffer overhead from LoRa.

    The second struct, `SensorVals`, represents the true values of each sensor.
    In particular, the raw CAN shorts representing floats are converted back to `f32`,
    and some integral shorts are cast to ints (`i32`). This struct has its own
    constructor to encapsulate that process.

- `sensor_list.json`: Contains the reference JSON object of each sensor and its
    type/formatting information. The JSON is formatted as follows:

    - Each key on the surface represents a sensor. This excludes LoRa data outside
      of the CAN, such as a packetnumber or signal information.

    - Each sensor has a key `type` with their true datatype (in 32-bit),
      and a key `bias` for their true starting value (added to their raw value)

    - For sensors that have type `float`, an additional key `scale` is added
      to represent the decimal place their base starts at; this is multiplied
      to the raw signal.