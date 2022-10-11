// RX using RF95, parsing data sent from TX
// Adapted from previous can_teensy_test project
// and the LoRa test files found in telemetry-firmware 23:
// https://github.com/NU-Formula-Racing/telemetry-firmware-23/blob/main/src/

using namespace std;

// Proper compilation
#include <Arduino.h>

// LoRa implementation libraries
#include <SPI.h>
#include <RH_RF95.h>

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

// RF95 inits and defines
#define RFM95_CS 10
#define RFM95_RST 2
#define RFM95_INT 3

// Must match RX freq; alt: 434.0
#define RF95_FREQ 915.0

// Singleton instance of driver
RH_RF95 rf95(RFM95_CS, RFM95_INT);

// Blink on receipt
#define LED 13

void setup() {
    // Initialize Serial
    while(!Serial);
    Serial.begin(9600);
    delay(100);
    Serial.println("CAN-LoRa test: RX");

    // Initial RF ops
    pinMode(LED, OUTPUT);     
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

// Expected packet length
#define LEN 21

void loop() {
    // Initialize radio packet
    char packet[LEN];

    // Check radio status
    if (rf95.available()) {
        // Should be a message for us now   
        uint8_t buf[RH_RF95_MAX_MESSAGE_LEN];
        uint8_t len = sizeof(buf);
    
        // The buffer should match exactly the length of the message
        if (len == LEN || rf95.recv(buf, &len)) {
            // Receive successful
            digitalWrite(LED, HIGH);
            RH_RF95::printBuffer("Received ", buf, len);
            Serial.print("Got: ");
            Serial.println((char*) buf);
            // Serial.print("RSSI: ");
            // Serial.println(rf95.lastRssi(), DEC);

            // Parse data
            // Slice of packet to look into for parsing
            char short_buffer[2];

            // TODO: if possible, find a way to get slice without copying
            short_buffer[0] = packet[0];
            short_buffer[1] = packet[1];
            fl_wheel_speed = atoi(short_buffer);
            short_buffer[0] = packet[2];
            short_buffer[1] = packet[3];
            fl_brake_temperature = atoi(short_buffer);
            short_buffer[0] = packet[4];
            short_buffer[1] = packet[5];
            fr_wheel_speed = atoi(short_buffer);
            short_buffer[0] = packet[6];
            short_buffer[1] = packet[7];
            fr_brake_temperature = atoi(short_buffer);
            short_buffer[0] = packet[8];
            short_buffer[1] = packet[9];
            bl_wheel_speed = atoi(short_buffer);
            short_buffer[0] = packet[10];
            short_buffer[1] = packet[11];
            bl_brake_temperature = atoi(short_buffer);
            short_buffer[0] = packet[12];
            short_buffer[1] = packet[13];
            br_wheel_speed = atoi(short_buffer);
            short_buffer[0] = packet[14];
            short_buffer[1] = packet[15];
            br_brake_temperature = atoi(short_buffer);
            short_buffer[0] = packet[16];
            short_buffer[1] = packet[17];
            front_brake_pressure = atoi(short_buffer);
            short_buffer[0] = packet[18];
            short_buffer[1] = packet[19];
            rear_brake_pressure = atoi(short_buffer);

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

            // End receipt handling
            digitalWrite(LED, LOW);
        } else {
            Serial.println("Receive failed");
        }
    }
}