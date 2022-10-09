// TX using RF95 transferring the data from CAN to LoRa
// Adapted from previous can_teensy_test project
// and the LoRa test files found in telemetry-firmware 23:
// https://github.com/NU-Formula-Racing/telemetry-firmware-23/blob/main/src/

using namespace std;

// Proper compilation
#include <Arduino.h>

// CAN library for Teensy
#include "teensy_can.h"

// LoRa implementation libraries
#include <SPI.h>
#include <RH_RF95.h>

// Initialize bus
TeensyCAN<1> can_bus{};

// CAN data buffers
// Each signal is 16-bit with 10 sigs in total
// Total data: 160 bits = 32 bytes (chars)
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

// RF95 inits and defines
#define RFM95_CS 10
#define RFM95_RST 9
#define RFM95_INT 2

// Must match RX freq; alt: 434.0
#define RF95_FREQ 915.0

// Singleton instance of driver
RH_RF95 rf95(RFM95_CS, RFM95_INT);

void setup() {
    // Initialize CAN bus
    can_bus.RegisterRXMessage(fl_wheel_msg);
    can_bus.RegisterRXMessage(fr_wheel_msg);
    can_bus.RegisterRXMessage(bl_wheel_msg);
    can_bus.RegisterRXMessage(br_wheel_msg);
    can_bus.RegisterRXMessage(brake_pressure_msg);

    can_bus.Initialize(ICAN::BaudRate::kBaud1M);

    // Initialize Serial
    while(!Serial);
    Serial.begin(9600);
    delay(100);
    Serial.println("CAN-LoRa test: TX");

    // Initial RF ops
    pinMode(RFM95_RST, OUTPUT);
    digitalWrite(RFM95_RST, HIGH);

    // Manual reset
    digitalWrite(RFM95_RST, LOW);
    delay(10);
    digitalWrite(RFM95_RST, HIGH);
    delay(10);

    // Attempt to initialize radio
    while (!rf95.init()) {
        Serial.println("LoRa radio init failed");
        while (1); // "Terminates" program without halting, infinite loop
    }
    Serial.println("LoRa radio init OK");

    // Defaults after init are 434.0MHz, modulation GFSK_Rb250Fd250, +13dBm,
    // Bw = 125 kHz, Cr = 4/5, Sf = 128chips/symbol, CRC on

    // Attempt to set frequency as defined
    if (!rf95.setFrequency(RF95_FREQ)) {
        Serial.println("Failed to set frequency");
        while (1);
    }
    Serial.print("Set Freq to: "); Serial.println(RF95_FREQ);

    // Transmitter power can be set from 5 to 23 dBm
    rf95.setTxPower(23, false);

    Serial.println("Begin transmitting data");
    delay(500);
}

void loop() {
    // Update CAN data
    can_bus.Tick();

    // Print data to Serial
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

    // Initialize radio packet
    char packet[33];
    itoa(fl_wheel_speed, packet, 16);
    itoa(fl_brake_temperature, packet + 1, 16);
    itoa(fr_wheel_speed, packet + 2, 16);
    itoa(fr_brake_temperature, packet + 3, 16);
    itoa(bl_wheel_speed, packet + 4, 16);
    itoa(bl_brake_temperature, packet + 5, 16);
    itoa(br_wheel_speed, packet + 6, 16);
    itoa(br_brake_temperature, packet + 7, 16);
    itoa(front_brake_pressure, packet + 8, 16);
    itoa(rear_brake_pressure, packet + 9, 16);
    packet[32] = '\0';
    
    // Send data
    // Serial.print("Sending "); Serial.println(radiopacket);
    // Serial.println("Sending..."); delay(10);
    rf95.send((uint8_t *) packet, 20);

    // Wait for completion
    Serial.println("Waiting for packet to complete...");
    delay(10);
    rf95.waitPacketSent();
}