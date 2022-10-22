/********** INCLUDES **********/
#include <Arduino.h>
#include "telemetry.h"
#include "virtualTimer.h"

/********** DEFINES **********/
/**
 * Specify which of the device programs to compile:
 * - TX: receive from CAN, send to RX
 * - RX: receive from RX, sent through USB
 * Only uncomment ONE or undef stuff may happen!!!
 */

// #define TX
#define RX

/********** VARIABLES **********/
VirtualTimerGroup timer_group;

/********** PROGRAM **********/
void setup() {
  Serial.begin(9600);

  // Set up device and check success
  if (!telemetry_setup()) {
    Serial.println("Device setup failed");
    while(1) {}
  }

  // Add tasks to timer
  #ifdef TX
    timer_group.AddTimer(1U, tx_task);
  #endif

  #ifdef RX
    // Longer time increment used to give time to listen
    timer_group.AddTimer(1000U, rx_task);
  #endif
}

void loop() {
  // Tick once, check/update all groups within
  timer_group.Tick(millis());
}