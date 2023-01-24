/********** INCLUDES **********/
#include <Arduino.h>
#include "virtualTimer.h"

#include "telemetry.h"
#include "target.h"

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
  #ifdef TELEMETRY_BASE_STATION_TX
    // Serial.println("CAN-LoRa test: TX");
    timer_group.AddTimer(1500U, tx_tick_fast);
    timer_group.AddTimer(2000U, tx_tick_slow);
    timer_group.AddTimer(400U, tx_send);

    // timer_group.AddTimer(1U, tx_task);
  #endif

  #ifdef TELEMETRY_BASE_STATION_RX
    // Serial.println("CAN-LoRa test: RX");
    timer_group.AddTimer(400U, rx_task);
  #endif
}

void loop() {
  // Tick once, check/update all groups within
  timer_group.Tick(millis());
}
