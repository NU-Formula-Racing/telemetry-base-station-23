// Receive test (fake) data from an ESP32 in the CAN format and print to serial.
// This tests the functionality of PIO compilation as well as using the CAN library.

// If Arduino libraries aren't linked in native VS Code, this will complain about missing libraries
// However, as long as platform.ini has an Arduino framework configured, this can be ignored and the project will compile.
#include <Arduino.h>

void setup() {
    // Initialize Serial
    while(!Serial);
    Serial.begin(9600);
    delay(100);
    Serial.println!("Teensy CAN data test:")
}

void loop() {

}