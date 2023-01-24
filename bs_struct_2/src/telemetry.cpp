/**
 * @file telemetry.cpp
 * @author Chris Uustal, Derek Guo
 * @brief NFR Telemetry firmware TX and RX code
 * @version 2
 * @date 2022-10-22
 * 
 * @copyright Copyright (c) 2022
 * 
 */

/********** INCLUDES **********/
#include "telemetry.h"

#include "target.h"
#include "sensor_vals.h"
#include "ser_des.h"

#ifdef TELEMETRY_BASE_STATION_TX
  // CAN library for Teensy
  #include "teensy_can.h"
#endif

/********** DEFINES **********/

/* Packet size */
// Size of data packet to be sent over LoRa
// Math is conducted below:
#define PACKET_SIZE 23

/********** VARIABLES **********/

/* RadioHead */
// Driver
RH_RF95 rf95(RFM95_CS, RFM95_INT);

// Success
bool rfm95_init_successful = true;

#ifdef TELEMETRY_BASE_STATION_TX
  /* CAN signal */
  // Initialize bus
  TeensyCAN<CAN_BUS_LO> can_bus{};

  /* CAN data buffers */ 
  // Each signal is 16-bit with 10 sigs in total
  // Total data: 160 bits = 20 bytes (chars)
  CANSignal<float, 0, 16, CANTemplateConvertFloat(0.1), CANTemplateConvertFloat(0)> fl_wheel_speed_sig;
  CANSignal<float, 0, 16, CANTemplateConvertFloat(0.1), CANTemplateConvertFloat(0)> fr_wheel_speed_sig;
  CANSignal<float, 0, 16, CANTemplateConvertFloat(0.1), CANTemplateConvertFloat(0)> bl_wheel_speed_sig;
  CANSignal<float, 0, 16, CANTemplateConvertFloat(0.1), CANTemplateConvertFloat(0)> br_wheel_speed_sig;

  CANSignal<float, 16, 16, CANTemplateConvertFloat(0.1), CANTemplateConvertFloat(-40)> fl_brake_temperature_sig;
  CANSignal<float, 16, 16, CANTemplateConvertFloat(0.1), CANTemplateConvertFloat(-40)> fr_brake_temperature_sig;
  CANSignal<float, 16, 16, CANTemplateConvertFloat(0.1), CANTemplateConvertFloat(-40)> bl_brake_temperature_sig;
  CANSignal<float, 16, 16, CANTemplateConvertFloat(0.1), CANTemplateConvertFloat(-40)> br_brake_temperature_sig;

  CANRXMessage<2> fl_wheel_msg{can_bus, 0x400, fl_wheel_speed_sig, fl_brake_temperature_sig};
  CANRXMessage<2> fr_wheel_msg{can_bus, 0x401, fr_wheel_speed_sig, fr_brake_temperature_sig};
  CANRXMessage<2> bl_wheel_msg{can_bus, 0x402, bl_wheel_speed_sig, bl_brake_temperature_sig};
  CANRXMessage<2> br_wheel_msg{can_bus, 0x403, br_wheel_speed_sig, br_brake_temperature_sig};

  CANSignal<uint16_t, 0, 16, CANTemplateConvertFloat(1), CANTemplateConvertFloat(0)> front_brake_pressure_sig;
  CANSignal<uint16_t, 16, 16, CANTemplateConvertFloat(1), CANTemplateConvertFloat(0)> rear_brake_pressure_sig;

  CANRXMessage<2> brake_pressure_msg{can_bus, 0x410, front_brake_pressure_sig, rear_brake_pressure_sig};

  // Additional 3 bytes appended at end: 2 bytes for packetnum, 1 byte for null-terminator
  // Total packet size: 23 bytes < capacity

  /* Packet */
  // Common across send and receive functions
  char packet[PACKET_SIZE];
#endif

/* Data objects */

#ifdef TELEMETRY_BASE_STATION_TX
  // Slow updating data
  uint16_t slow_sensor_sig;

  // Control value
  uint8_t control;

  // Sensor value reference struct
  sensor_refs_t sensor_refs;
#endif

#ifdef TELEMETRY_BASE_STATION_RX
  // Sensor value data struct
  sensor_vals_t sensor_vals;
#endif

// Message code
message_code_t data_id = 0;

// Data buffer
uint8_t buf[RH_RF95_MAX_MESSAGE_LEN];

// Buffer length
#ifdef TELEMETRY_BASE_STATION_TX
  size_t buf_len = RH_RF95_MAX_MESSAGE_LEN;
#endif
#ifdef TELEMETRY_BASE_STATION_RX
  uint8_t buf_len = RH_RF95_MAX_MESSAGE_LEN;
#endif

// Packet number for ordering/debugging
// From header file
uint16_t packetnum = 0;

// Raw signal data
uint16_t fl_wheel_speed;
uint16_t fl_brake_temperature;
uint16_t fr_wheel_speed;
uint16_t fr_brake_temperature;
uint16_t bl_wheel_speed;
uint16_t bl_brake_temperature;
uint16_t br_wheel_speed;
uint16_t br_brake_temperature;

uint16_t front_brake_pressure;
uint16_t rear_brake_pressure;

#ifdef TELEMETRY_BASE_STATION_RX
  // The "true" values, or formatted values for floating-point sensors
  float fl_wheel_speed_true;
  float fl_brake_temperature_true;
  float fr_wheel_speed_true;
  float fr_brake_temperature_true;
  float bl_wheel_speed_true;
  float bl_brake_temperature_true;
  float br_wheel_speed_true;
  float br_brake_temperature_true;
#endif

#ifdef TELEMETRY_BASE_STATION_TX
  /* Flags */
  flag_t slow_flag = 0;
  // flag_t med_flag = 0;
  flag_t fast_flag = 0;
  uint32_t cond_flag = 0;
#endif

/********** PUBLIC FUNCTION DEFINITIONS **********/

/**
 * @brief Runs all setup functions for the RFM95 module 
 * @return true if setup was successful
 * @return false if setup was unsuccessful
 */
bool telemetry_setup() {
  pinMode(RFM95_RST, OUTPUT);
  digitalWrite(RFM95_RST, HIGH);

  // Manual reset
  digitalWrite(RFM95_RST, LOW);
  delay(10);
  digitalWrite(RFM95_RST, HIGH);
  delay(10);

  // Set up RadioHead
  if (rf95.init() == true) {
    // Defaults after init are 434.0MHz, modulation GFSK_Rb250Fd250, +13dbM
    if (rf95.setFrequency(RF95_FREQ) == true) {
      // Defaults after init are 434.0MHz, 13dBm, Bw = 125 kHz, Cr = 4/5, Sf = 128chips/symbol, CRC on

      // The default transmitter power is 13dBm, using PA_BOOST.
      // If you are using RFM95/96/97/98 modules which uses the PA_BOOST transmitter pin, then 
      // you can set transmitter powers from 5 to 23 dBm:
      rf95.setTxPower(23, false);
    } else {
      // Serial.println("setFrequency failed");
      rfm95_init_successful = false;
    }
  } else {
    // Serial.println("LoRa radio init failed");
    rfm95_init_successful = false;
  }

  #ifdef TELEMETRY_BASE_STATION_TX
    // Initialize CAN bus
    can_bus.RegisterRXMessage(fl_wheel_msg);
    can_bus.RegisterRXMessage(fr_wheel_msg);
    can_bus.RegisterRXMessage(bl_wheel_msg);
    can_bus.RegisterRXMessage(br_wheel_msg);
    can_bus.RegisterRXMessage(brake_pressure_msg);

    can_bus.Initialize(ICAN::BaudRate::kBaud1M);

    // Get and store sensor references
    sensor_refs = {
      .fast = {
        .fl_wheel_speed = &(fl_wheel_speed_sig.value_ref()),
        .fl_brake_temperature = &(fl_brake_temperature_sig.value_ref()),
        .fr_wheel_speed = &(fr_wheel_speed_sig.value_ref()),
        .fr_brake_temperature = &(fr_brake_temperature_sig.value_ref()),
        .bl_wheel_speed = &(bl_wheel_speed_sig.value_ref()),
        .bl_brake_temperature = &(bl_brake_temperature_sig.value_ref()),
        .br_wheel_speed = &(br_wheel_speed_sig.value_ref()),
        .br_brake_temperature = &(br_brake_temperature_sig.value_ref()),
        .front_brake_pressure = &(front_brake_pressure_sig.value_ref()),
        .rear_brake_pressure = &(rear_brake_pressure_sig.value_ref()),
        .packetnum = &packetnum
      },
      .slow = {
        .fake_value = &slow_sensor_sig
      },
      .control = &control
    };

    // Reset control
    control = 0;
  #endif

  return rfm95_init_successful;
}

/*** TRANSCEIVER CODE ***/

/**
 * @brief Update sensors deemed "fast"
 * 
 */
void tx_tick_fast() {
  #ifdef TELEMETRY_BASE_STATION_TX
    can_bus.Tick();
    ++packetnum;
    fast_flag = 1;
  #endif
}

/**
 * @brief Update sensors deemed "slow"
 * 
 */
void tx_tick_slow() {
  #ifdef TELEMETRY_BASE_STATION_TX
    ++slow_sensor_sig;
    slow_flag = 1;
  #endif
}

/**
 * @brief Transmit data
 * 
 */
void tx_send() {
  if (rfm95_init_successful == true) {
    #ifdef TELEMETRY_BASE_STATION_TX
      // From the flags, transcribe them into a singular code, then reset flags
      get_msg_code(&data_id, &fast_flag, &slow_flag, &cond_flag);
      
      // // Serialize the data
      // serialize(&data_id, &sensor_refs, buf, &buf_len);

    buf[0] = (uint8_t) data_id;
    buf_len = 1;
    // buf[buf_len++] = 'a'; buf[buf_len++] = 'b';
    if (data_id & 0b1) {
      buf[buf_len++] = 'a';
    }
    if (data_id & 0b100) {
      buf[buf_len++] = 'b';
    }

      // Send data and verify completion
      delay(10);
      rf95.send(buf, buf_len);
      delay(10);
      rf95.waitPacketSent();
    #endif
  }
}

/**
 * @brief Transceiver (TX) code
 * 
 */
// void tx_task() {
//   if (rfm95_init_successful == true) {
//     #ifdef TELEMETRY_BASE_STATION_TX
//       // Update CAN data
//       can_bus.Tick();

//       // Re-encode floats to 

//       // Test: print data to Serial
//       // Serial.print("Sending WS { FL: "); Serial.print(float(fl_wheel_speed_sig));
//       // Serial.print(" FR: "); Serial.print(float(fr_wheel_speed_sig));
//       // Serial.print(" BL: "); Serial.print(float(bl_wheel_speed_sig));
//       // Serial.print(" BR: "); Serial.print(float(br_wheel_speed_sig));
//       // Serial.print(" } BT { FL: "); Serial.print(float(fl_brake_temperature_sig));
//       // Serial.print(" FR: "); Serial.print(float(fr_brake_temperature_sig));
//       // Serial.print(" BL: "); Serial.print(float(bl_brake_temperature_sig));
//       // Serial.print(" BR: "); Serial.print(float(br_brake_temperature_sig));
//       // Serial.print(" } BP: { F: "); Serial.print(uint16_t(front_brake_pressure_sig));
//       // Serial.print(" R: "); Serial.print(uint16_t(rear_brake_pressure_sig));
//       // Serial.print(" } #"); Serial.println(packetnum);

//       // Convert chars to int
//       ftos(&(fl_wheel_speed_sig.value_ref()), &fl_wheel_speed, 10.0, 0.0);
//       stob((char*) &fl_wheel_speed, packet);
//       ftos(&(fl_brake_temperature_sig.value_ref()), &fl_brake_temperature, 10.0, -40.0);
//       stob((char*) &fl_brake_temperature, packet + 2);
//       ftos(&(fr_wheel_speed_sig.value_ref()), &fr_wheel_speed, 10.0, 0.0);
//       stob((char*) &fr_wheel_speed, packet + 4);
//       ftos(&(fr_brake_temperature_sig.value_ref()), &fr_brake_temperature, 10.0, -40.0);
//       stob((char*) &fr_brake_temperature, packet + 6);
//       ftos(&(bl_wheel_speed_sig.value_ref()), &bl_wheel_speed, 10.0, 0.0);
//       stob((char*) &bl_wheel_speed, packet + 8);
//       ftos(&(bl_brake_temperature_sig.value_ref()), &bl_brake_temperature, 10.0, -40.0);
//       stob((char*) &bl_brake_temperature, packet + 10);
//       ftos(&(br_wheel_speed_sig.value_ref()), &br_wheel_speed, 10.0, 0.0);
//       stob((char*) &br_wheel_speed, packet + 12);
//       ftos(&(br_brake_temperature_sig.value_ref()), &br_brake_temperature, 10.0, -40.0);
//       stob((char*) &br_brake_temperature, packet + 14);

//       stob((char*) &(front_brake_pressure_sig.value_ref()), packet + 16);
//       stob((char*) &(rear_brake_pressure_sig.value_ref()), packet + 18);

//       stob((char*) &packetnum, packet + 20);
//       packetnum++;
    
//       packet[22] = '\0';

//       // Serial.print("Packet: "); Serial.println(packet);
//       RH_RF95::printBuffer("Packet ", (uint8_t*) packet, PACKET_SIZE);
      
//       // Send data and verify completion
//       delay(10);
//       rf95.send((uint8_t *) packet, PACKET_SIZE);
//       delay(10);
//       rf95.waitPacketSent();
//     #endif
//   }
// }

/*** TRANSCEIVER CODE ***/

/**
 * @brief 
 * 
 */
void rx_task() {
  if (rf95.available() && (rfm95_init_successful == true)) {
    #ifdef TELEMETRY_BASE_STATION_RX
      // Check if a message has been received, and retrieve its data and length
      if (rf95.recv(buf, &buf_len)) {
        // Receive successful
        RH_RF95::printBuffer("Received ", buf, buf_len);
        Serial.print("Got: "); Serial.println((char*) buf);
        // if (buf[0] & 0b1)
        //   Serial.println("RD");

        // Parse data
        deserialize(&data_id, &sensor_vals, buf);

        // Print data to Serial
        Serial.print("WS { FL: "); Serial.print(sensor_vals.fast.fl_wheel_speed);
        Serial.print(" FR: "); Serial.print(sensor_vals.fast.fr_wheel_speed);
        Serial.print(" BL: "); Serial.print(sensor_vals.fast.bl_wheel_speed);
        Serial.print(" BR: "); Serial.print(sensor_vals.fast.br_wheel_speed);
        Serial.print(" BT { FL: "); Serial.print(sensor_vals.fast.fl_brake_temperature);
        Serial.print(" FR: "); Serial.print(sensor_vals.fast.fr_brake_temperature);
        Serial.print(" BL: "); Serial.print(sensor_vals.fast.bl_brake_temperature);
        Serial.print(" BR: "); Serial.print(sensor_vals.fast.br_brake_temperature);
        Serial.print(" } BP: { F: "); Serial.print(sensor_vals.fast.front_brake_pressure);
        Serial.print(" R: "); Serial.print(sensor_vals.fast.rear_brake_pressure);
        Serial.print(" } SL: { D: "); Serial.print(sensor_vals.slow.fake_value);
        Serial.print(" } Ct: "); Serial.print(sensor_vals.control);
        Serial.print(" , #"); Serial.println(sensor_vals.fast.packetnum);
      } else {
        Serial.println("Receive failed");
      }
    #endif
  }
}

/**
 * @brief Receiver (RX) code
 * 
 */
// void rx_task() {
//   if (rf95.available() && (rfm95_init_successful == true)) {

//     // Should be a message for us now   
//     uint8_t packet[RH_RF95_MAX_MESSAGE_LEN];
//     uint8_t len = sizeof(packet);

//     // The buffer should match exactly the length of the message
//     if (rf95.recv(packet, &len)) {
//       // Receive successful
//       RH_RF95::printBuffer("Received ", packet, len);
//       Serial.print("Got: ");
//       Serial.println((char*) packet);

//       #ifdef TELEMETRY_BASE_STATION_RX
//         // Parse data
//         btos((char*) &fl_wheel_speed, (char*) packet);
//         stof(&fl_wheel_speed_true, &fl_wheel_speed, 10.0, 0.0);
//         btos((char*) &fl_brake_temperature, (char*) packet + 2);
//         stof(&fl_brake_temperature_true, &fl_brake_temperature, 10.0, -40.0);
//         btos((char*) &fr_wheel_speed, (char*) packet + 4);
//         stof(&fr_wheel_speed_true, &fr_wheel_speed, 10.0, 0.0);
//         btos((char*) &fr_brake_temperature, (char*) packet + 6);
//         stof(&fr_brake_temperature_true, &fr_brake_temperature, 10.0, -40.0);
//         btos((char*) &bl_wheel_speed, (char*) packet + 8);
//         stof(&bl_wheel_speed_true, &bl_wheel_speed, 10.0, 0.0);
//         btos((char*) &bl_brake_temperature, (char*) packet + 10);
//         stof(&bl_brake_temperature_true, &bl_brake_temperature, 10.0, -40.0);
//         btos((char*) &br_wheel_speed, (char*) packet + 12);
//         stof(&br_wheel_speed_true, &br_wheel_speed, 10.0, 0.0);
//         btos((char*) &br_brake_temperature, (char*) packet + 14);
//         stof(&br_brake_temperature_true, &br_brake_temperature, 10.0, -40.0);

//         btos((char*) &front_brake_pressure, (char*) packet + 16);
//         btos((char*) &rear_brake_pressure, (char*) packet + 18);

//         btos((char*) &packetnum, (char*) packet + 20);

//         // Print data to Serial
//         Serial.print("WS { FL: "); Serial.print(fl_wheel_speed_true);
//         Serial.print(" FR: "); Serial.print(fr_wheel_speed_true);
//         Serial.print(" BL: "); Serial.print(bl_wheel_speed_true);
//         Serial.print(" BR: "); Serial.print(br_wheel_speed_true);
//         Serial.print(" } BT { FL: "); Serial.print(fl_brake_temperature_true);
//         Serial.print(" FR: "); Serial.print(fr_brake_temperature_true);
//         Serial.print(" BL: "); Serial.print(bl_brake_temperature_true);
//         Serial.print(" BR: "); Serial.print(br_brake_temperature_true);
//         Serial.print(" } BP: { F: "); Serial.print(front_brake_pressure);
//         Serial.print(" R: "); Serial.print(rear_brake_pressure);
//         Serial.print(" } #"); Serial.println(packetnum);
//       #endif
//     } else {
//       Serial.println("Receive failed");
//     }
//   }
// }