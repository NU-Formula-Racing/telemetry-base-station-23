/**
 * @file telemetry.cpp
 * @author Chris Uustal
 * @brief NFR Telemetry firmware TX and RX code
 * @version 1
 * @date 2022-10-20
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
#endif

/********** VARIABLES **********/
RH_RF95 rf95(RFM95_CS, RFM95_INT);
int16_t packetnum = 0;
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

  return rfm95_init_successful;
}

/**
 * @brief Transceiver (TX) code
 * 
 */

void tx_task() {
  if (rf95.available() && (rfm95_init_successful == true)) {

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

    // Should be a message for us now   
    uint8_t buf[RH_RF95_MAX_MESSAGE_LEN];
    uint8_t len = sizeof(buf);
    
    if (rf95.recv(buf, &len)) {
      RH_RF95::printBuffer("Received: ", buf, len);
      Serial.print("Got: ");
      Serial.println((char*)buf);
      Serial.print("RSSI: ");
      Serial.println(rf95.lastRssi(), DEC);
      
      // Send a reply
      uint8_t data[] = "And hello back to you";
      // Delay responding--otherwise you can respond too fast and it doesn't hear you
      delay(10);
      rf95.send(data, sizeof(data));
      rf95.waitPacketSent();
      Serial.println("Sent a reply");
    } else {
      Serial.println("Receive failed");
    }
  }
}

/**
 * @brief Receiver (RX) code
 * 
 */
void rx_task() {
  if (rfm95_init_successful == true) {
    // Serial.println("Sending to rf95_server");
    // Send a message to TX
    
    char radiopacket[20] = "Hello World #      ";
    itoa(packetnum++, radiopacket+13, 10);
    Serial.print("Sending "); 
    Serial.println(radiopacket);
    radiopacket[19] = 0;
    
    // Serial.println("Sending..."); 
    delay(10);
    rf95.send((uint8_t *)radiopacket, 20);

    // Serial.println("Waiting for packet to complete..."); 
    delay(10);
    rf95.waitPacketSent();
    // Now wait for a reply
    uint8_t buf[RH_RF95_MAX_MESSAGE_LEN];
    uint8_t len = sizeof(buf);

    Serial.println("Waiting for reply..."); 
    delay(10);
    if (rf95.waitAvailableTimeout(1000)) { 
      // Should be a reply message for us now   
      if (rf95.recv(buf, &len)) {
        Serial.print("Got reply: ");
        Serial.println((char*)buf);
        Serial.print("RSSI: ");
        Serial.println(rf95.lastRssi(), DEC);    
      } else {
        Serial.println("Receive failed");
      }
    } else {
      Serial.println("No reply, is there a listener around?");
    }
  }
}