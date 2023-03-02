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
#include "ser_des.h"

#ifdef TELEMETRY_BASE_STATION_TX
  // CAN library for Teensy
  #include "teensy_can.h"
#endif

/********** DEFINES **********/


/********** VARIABLES **********/

/* RadioHead */
// Driver
RH_RF95 rf95(RFM95_CS, RFM95_INT);

// Packet number for ordering/debugging
int32_t packetnum = 0;
uint8_t* visitor;

// Success
bool rfm95_init_successful = true;

#ifdef TELEMETRY_BASE_STATION_TX
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
#endif

/* Packet */
// Common across send and receive functions
uint8_t packet[RH_RF95_MAX_MESSAGE_LEN];

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
  #endif

  #ifdef TELEMETRY_BASE_STATION_RX
    // Dummy values; test if current pipeline allows for RX comp
    // fl_wheel_speed = 10.0;
    // fl_brake_temperature = 1.0;
    // fr_wheel_speed = 2.0;
    // fr_brake_temperature = 3.0;
    // bl_wheel_speed = 4.0;
    // bl_brake_temperature = 5.0;
    // br_wheel_speed = 6.0;
    // br_brake_temperature = 7.0;

    // front_brake_pressure = 8;
    // rear_brake_pressure = 9;
  #endif

  return rfm95_init_successful;
}

/**
 * @brief Transceiver (TX) code
 * 
 */
void tx_task() {
  if (rfm95_init_successful == true) {
    #ifdef TELEMETRY_BASE_STATION_TX
      // Update CAN data
      can_bus.Tick();

      // Re-encode floats to 
      visitor = (uint8_t*) &packetnum;
      packet[0] = *(visitor++);
      packet[1] = *(visitor++);
      packet[2] = *(visitor++);
      packet[3] = *(visitor++);

      // Test: print data to Serial
      // Serial.print("Sending WS { FL: "); Serial.print(float(fl_wheel_speed_sig));
      // Serial.print(" FR: "); Serial.print(float(fr_wheel_speed_sig));
      // Serial.print(" BL: "); Serial.print(float(bl_wheel_speed_sig));
      // Serial.print(" BR: "); Serial.print(float(br_wheel_speed_sig));
      // Serial.print(" } BT { FL: "); Serial.print(float(fl_brake_temperature_sig));
      // Serial.print(" FR: "); Serial.print(float(fr_brake_temperature_sig));
      // Serial.print(" BL: "); Serial.print(float(bl_brake_temperature_sig));
      // Serial.print(" BR: "); Serial.print(float(br_brake_temperature_sig));
      // Serial.print(" } BP: { F: "); Serial.print(uint16_t(front_brake_pressure_sig));
      // Serial.print(" R: "); Serial.print(uint16_t(rear_brake_pressure_sig));
      // Serial.print(" } #"); Serial.println(packetnum);

      // Convert chars to int
      // ftos(&(fl_wheel_speed_sig.value_ref()), &fl_wheel_speed, 10.0, 0.0);
      // stob((char*) &fl_wheel_speed, packet);
      // ftos(&(fl_brake_temperature_sig.value_ref()), &fl_brake_temperature, 10.0, -40.0);
      // stob((char*) &fl_brake_temperature, packet + 2);
      // ftos(&(fr_wheel_speed_sig.value_ref()), &fr_wheel_speed, 10.0, 0.0);
      // stob((char*) &fr_wheel_speed, packet + 4);
      // ftos(&(fr_brake_temperature_sig.value_ref()), &fr_brake_temperature, 10.0, -40.0);
      // stob((char*) &fr_brake_temperature, packet + 6);
      // ftos(&(bl_wheel_speed_sig.value_ref()), &bl_wheel_speed, 10.0, 0.0);
      // stob((char*) &bl_wheel_speed, packet + 8);
      // ftos(&(bl_brake_temperature_sig.value_ref()), &bl_brake_temperature, 10.0, -40.0);
      // stob((char*) &bl_brake_temperature, packet + 10);
      // ftos(&(br_wheel_speed_sig.value_ref()), &br_wheel_speed, 10.0, 0.0);
      // stob((char*) &br_wheel_speed, packet + 12);
      // ftos(&(br_brake_temperature_sig.value_ref()), &br_brake_temperature, 10.0, -40.0);
      // stob((char*) &br_brake_temperature, packet + 14);

      // stob((char*) &(front_brake_pressure_sig.value_ref()), packet + 16);
      // stob((char*) &(rear_brake_pressure_sig.value_ref()), packet + 18);

      // stob((char*) &packetnum, packet + 20);
      packetnum++;

      // Serial.print("Packet: "); Serial.println(packet);
      // RH_RF95::printBuffer("Packet ", (uint8_t*) packet, PACKET_SIZE);
      
      // Send data and verify completion
      // delay(10);
      rf95.send(packet, 4);
      // delay(10);
      rf95.waitPacketSent();
    #endif
  }
}

/**
 * @brief Receiver (RX) code
 * 
 */
void rx_task() {
  if (rf95.available() && (rfm95_init_successful == true)) {

    // Should be a message for us now   
    uint8_t len = sizeof(packet);

    // The buffer should match exactly the length of the message
    if (rf95.recv(packet, &len)) {
      // Receive successful
      // RH_RF95::printBuffer("Received ", packet, len);
      // Serial.println(len);

      // Serial.print("Got: ");
      visitor = (uint8_t*) &packetnum;
      *(visitor++) = packet[0];
      *(visitor++) = packet[1];
      *(visitor++) = packet[2];
      *(visitor++) = packet[3];
      Serial.println(packetnum);

      // Serial.println((char*) packet);

      // #ifdef TELEMETRY_BASE_STATION_RX
      //   // Parse data
      //   btos((char*) &fl_wheel_speed, (char*) packet);
      //   stof(&fl_wheel_speed_true, &fl_wheel_speed, 10.0, 0.0);
      //   btos((char*) &fl_brake_temperature, (char*) packet + 2);
      //   stof(&fl_brake_temperature_true, &fl_brake_temperature, 10.0, -40.0);
      //   btos((char*) &fr_wheel_speed, (char*) packet + 4);
      //   stof(&fr_wheel_speed_true, &fr_wheel_speed, 10.0, 0.0);
      //   btos((char*) &fr_brake_temperature, (char*) packet + 6);
      //   stof(&fr_brake_temperature_true, &fr_brake_temperature, 10.0, -40.0);
      //   btos((char*) &bl_wheel_speed, (char*) packet + 8);
      //   stof(&bl_wheel_speed_true, &bl_wheel_speed, 10.0, 0.0);
      //   btos((char*) &bl_brake_temperature, (char*) packet + 10);
      //   stof(&bl_brake_temperature_true, &bl_brake_temperature, 10.0, -40.0);
      //   btos((char*) &br_wheel_speed, (char*) packet + 12);
      //   stof(&br_wheel_speed_true, &br_wheel_speed, 10.0, 0.0);
      //   btos((char*) &br_brake_temperature, (char*) packet + 14);
      //   stof(&br_brake_temperature_true, &br_brake_temperature, 10.0, -40.0);

      //   btos((char*) &front_brake_pressure, (char*) packet + 16);
      //   btos((char*) &rear_brake_pressure, (char*) packet + 18);

      //   btos((char*) &packetnum, (char*) packet + 20);

      //   // Print data to Serial
      //   Serial.print("WS { FL: "); Serial.print(fl_wheel_speed_true);
      //   Serial.print(" FR: "); Serial.print(fr_wheel_speed_true);
      //   Serial.print(" BL: "); Serial.print(bl_wheel_speed_true);
      //   Serial.print(" BR: "); Serial.print(br_wheel_speed_true);
      //   Serial.print(" } BT { FL: "); Serial.print(fl_brake_temperature_true);
      //   Serial.print(" FR: "); Serial.print(fr_brake_temperature_true);
      //   Serial.print(" BL: "); Serial.print(bl_brake_temperature_true);
      //   Serial.print(" BR: "); Serial.print(br_brake_temperature_true);
      //   Serial.print(" } BP: { F: "); Serial.print(front_brake_pressure);
      //   Serial.print(" R: "); Serial.print(rear_brake_pressure);
      //   Serial.print(" } #"); Serial.println(packetnum);
      // #endif
    } else {
      Serial.println("Receive failed");
    }
  }
}