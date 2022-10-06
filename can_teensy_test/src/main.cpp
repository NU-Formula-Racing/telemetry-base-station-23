// Receive test (fake) data from an ESP32 in the CAN format and print to serial.
// This tests the functionality of PIO compilation as well as using the CAN library.

using namespace std;

// If Arduino libraries aren't linked in native VS Code, this may complain about missing libraries
// However, as long as platform.ini has an Arduino framework configured, this can be ignored and the project will compile.
#include <Arduino.h>

// From custom CAN lirary, must include to build for Teensy properly
#include "teensy_can.h"

// Initialize CAN bus, number 1
TeensyCAN<1> can_bus{};

// Signal instantiation to be used for messages
// Each signal is to be used for 1 message, and is constructed via template class
// These signals are labeled for reception (RX)
CANSignal<float, 0, 16, CANTemplateConvertFloat(0.01), CANTemplateConvertFloat(0), true> float_rx_sig{};
CANSignal<uint8_t, 16, 8, CANTemplateConvertFloat(1), CANTemplateConvertFloat(0), false> uint8_t_rx_sig{};
CANSignal<bool, 24, 1, CANTemplateConvertFloat(1), CANTemplateConvertFloat(0), false> bool_rx_sig{};
CANSignal<uint32_t, 32, 32, CANTemplateConvertFloat(1), CANTemplateConvertFloat(0), false> ms_rx_sig{};

// Message objects
// Number of signals is templated arg, constructor takes bus, a message ID, and the signal objects (variable length)
CANRXMessage<4> rx_msg{can_bus, 0x200, float_rx_sig, uint8_t_rx_sig, bool_rx_sig, ms_rx_sig};

void setup() {
    // Initialize CAN bus with RX message (defined above) and standardized baud rate (from library)
    can_bus.RegisterRXMessage(rx_msg);
    can_bus.Initialize(ICAN::BaudRate::kBaud1M);

    // Initialize Serial
    while(!Serial);
    Serial.begin(9600);
    delay(100);
    Serial.println("Teensy CAN data test:");
}

void loop() {
    // Get data from signals
    float rx_float = float_rx_sig;
    uint8_t rx_uint8_t = uint8_t_rx_sig;
    bool rx_bool = bool_rx_sig;
    int32_t rx_ms = ms_rx_sig;

    // rx_msg automatically updates from interrupt, can be printed from immediately
    Serial.print("Received float ");
    Serial.print(rx_float);
    Serial.print(", uint8_t ");
    Serial.print(rx_uint8_t);
    Serial.print(", bool ");
    Serial.print(rx_bool);
    Serial.print(", time ");
    Serial.println(rx_ms);
}