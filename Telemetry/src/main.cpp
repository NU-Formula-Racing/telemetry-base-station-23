#include <Arduino.h>
#include "telemetry.h"
#include "virtualTimer.h"

// #define SERVER
#define CLIENT

VirtualTimerGroup timer_group;

void setup() 
{
  Serial.begin(9600);

  telemetry_setup();

  #ifdef CLIENT
  timer_group.AddTimer(1000U, client_task);
  #endif
  
  #ifdef SERVER
  timer_group.AddTimer(1U, server_task);
  #endif
}

void loop()
{
  timer_group.Tick(millis());
}