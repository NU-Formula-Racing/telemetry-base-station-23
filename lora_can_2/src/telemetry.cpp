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

#ifdef TELEMETRY_BASE_STATION_TX
  // CAN library for Teensy
  #include "teensy_can.h"
  #include "ser_des.h"
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

// Packet number for ordering/debugging
int16_t packetnum = 0;

// Success
bool rfm95_init_successful = true;

#ifdef TELEMETRY_BASE_STATION_TX
  // Initialize bus
  TeensyCAN<1> can_bus{};

  /* CAN data buffers */ 
  // Each signal is 16-bit with 10 sigs in total
  // Total data: 160 bits = 20 bytes (chars)
  CANSignal<float, 0, 16, CANTemplateConvertFloat(0.1), CANTemplateConvertFloat(0)> fl_wheel_speed;
  CANSignal<float, 0, 16, CANTemplateConvertFloat(0.1), CANTemplateConvertFloat(0)> fr_wheel_speed;
  CANSignal<float, 0, 16, CANTemplateConvertFloat(0.1), CANTemplateConvertFloat(0)> bl_wheel_speed;
  CANSignal<float, 0, 16, CANTemplateConvertFloat(0.1), CANTemplateConvertFloat(0)> br_wheel_speed;

  CANSignal<float, 16, 16, CANTemplateConvertFloat(0.1), CANTemplateConvertFloat(-40)> fl_brake_temperature;
  CANSignal<float, 16, 16, CANTemplateConvertFloat(0.1), CANTemplateConvertFloat(-40)> fr_brake_temperature;
  CANSignal<float, 16, 16, CANTemplateConvertFloat(0.1), CANTemplateConvertFloat(-40)> bl_brake_temperature;
  CANSignal<float, 16, 16, CANTemplateConvertFloat(0.1), CANTemplateConvertFloat(-40)> br_brake_temperature;

  CANRXMessage<2> fl_wheel_msg{can_bus, 0x400, fl_wheel_speed, fl_brake_temperature};
  CANRXMessage<2> fr_wheel_msg{can_bus, 0x401, fr_wheel_speed, fr_brake_temperature};
  CANRXMessage<2> bl_wheel_msg{can_bus, 0x402, bl_wheel_speed, bl_brake_temperature};
  CANRXMessage<2> br_wheel_msg{can_bus, 0x403, br_wheel_speed, br_brake_temperature};

  CANSignal<uint16_t, 0, 16, CANTemplateConvertFloat(1), CANTemplateConvertFloat(0)> front_brake_pressure;
  CANSignal<uint16_t, 16, 16, CANTemplateConvertFloat(1), CANTemplateConvertFloat(0)> rear_brake_pressure;

  CANRXMessage<2> brake_pressure_msg{can_bus, 0x410, front_brake_pressure, rear_brake_pressure};

  // Additional 3 bytes appended at end: 2 bytes for packetnum, 1 byte for null-terminator
  // Total packet size: 23 bytes < capacity

  /* Packet */
  // Common across send and receive functions
  char packet[PACKET_SIZE];
#endif

#ifdef TELEMETRY_BASE_STATION_RX
  // Formatted data
  float fl_wheel_speed;
  float fl_brake_temperature;
  float fr_wheel_speed;
  float fr_brake_temperature;
  float bl_wheel_speed;
  float bl_brake_temperature;
  float br_wheel_speed;
  float br_brake_temperature;

  uint16_t front_brake_pressure;
  uint16_t rear_brake_pressure;
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
    fl_wheel_speed = 0.0;;
    fl_brake_temperature = 1.0;;
    fr_wheel_speed = 2.0;
    fr_brake_temperature = 3.0;
    bl_wheel_speed = 4.0;
    bl_brake_temperature = 5.0;
    br_wheel_speed = 6.0;
    br_brake_temperature = 7.0;

    front_brake_pressure = 8;
    rear_brake_pressure = 9;
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

      // Test: print data to Serial
      Serial.print("Sending WS { FL: "); Serial.print(fl_wheel_speed);
      Serial.print(" FR: "); Serial.print(fr_wheel_speed);
      Serial.print(" BL: "); Serial.print(bl_wheel_speed);
      Serial.print(" BR: "); Serial.print(br_wheel_speed);
      Serial.print(" } BT { FL: "); Serial.print(fl_brake_temperature);
      Serial.print(" FR: "); Serial.print(fr_brake_temperature);
      Serial.print(" BL: "); Serial.print(bl_brake_temperature);
      Serial.print(" BR: "); Serial.print(br_brake_temperature);
      Serial.print(" } BP: { F: "); Serial.print(front_brake_pressure);
      Serial.print(" R: "); Serial.print(rear_brake_pressure);
      Serial.println(" }");
    #endif

    char radiopacket[20] = "Hello World #      ";
    #ifdef TELEMETRY_BASE_STATION_TX
      stob((char*) &packetnum, radiopacket + 13);
      ++packetnum;
    #endif
    Serial.print("Sending "); 
    Serial.println(radiopacket);
    radiopacket[19] = 0;
    
    // Serial.println("Sending..."); 
    delay(10);
    rf95.send((uint8_t *)radiopacket, 20);

    // Serial.println("Waiting for packet to complete..."); 
    delay(10);
    rf95.waitPacketSent();
  }
}

/**
 * @brief Receiver (RX) code
 * 
 */
void rx_task() {
  if (rf95.available() && (rfm95_init_successful == true)) {

    // Should be a message for us now   
    uint8_t buf[RH_RF95_MAX_MESSAGE_LEN];
    uint8_t len = sizeof(buf);
    
    if (rf95.recv(buf, &len)) {
      RH_RF95::printBuffer("Received: ", buf, len);
      Serial.print("Got: ");
      Serial.println((char*)buf);
      Serial.print("RSSI: ");
      Serial.println(rf95.lastRssi(), DEC);
    } else {
      Serial.println("Receive failed");
    }

    #ifdef TELEMETRY_BASE_STATION_RX
      // Print data to Serial
      Serial.print("WS { FL: "); Serial.print(fl_wheel_speed);
      Serial.print(" FR: "); Serial.print(fr_wheel_speed);
      Serial.print(" BL: "); Serial.print(bl_wheel_speed);
      Serial.print(" BR: "); Serial.print(br_wheel_speed);
      Serial.print(" } BT { FL: "); Serial.print(fl_brake_temperature);
      Serial.print(" FR: "); Serial.print(fr_brake_temperature);
      Serial.print(" BL: "); Serial.print(bl_brake_temperature);
      Serial.print(" BR: "); Serial.print(br_brake_temperature);
      Serial.print(" } BP: { F: "); Serial.print(front_brake_pressure);
      Serial.print(" R: "); Serial.print(rear_brake_pressure);
      Serial.println(" }");
    #endif
  }
}