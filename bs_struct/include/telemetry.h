/**
 * @file telemetry.h
 * @author Chris Uustal, Derek Guo
 * @brief Header file for telemetry firmware server (TX) and client (RX) functions
 * @version 1
 * @date 2022-10-20
 * 
 * @copyright Copyright (c) 2022
 * 
 */

#ifndef TELEMETRY_H
#define TELEMETRY_H

/********** INCLUDES **********/
#include <Arduino.h>
#include <SPI.h>
#include <RH_RF95.h>

/********** DEFINES **********/
#define RFM95_CS 10
#define RFM95_RST 2
#define RFM95_INT 3
#define RF95_FREQ 915.0

/********** STRUCTS **********/
#pragma pack(2)
typedef struct CAN_DATA {
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
  float garbage_fl_val;
  uint16_t packetnum;
  char signal_data;
} can_data_t;

/********** VARIABLES **********/
// Singleton instance of the radio driver
extern RH_RF95 rf95;

extern bool rfm95_init_successful;

extern int16_t packetnum;  // packet counter, we increment per xmission

/********** PUBLIC FUNCTION PROTOTYPES **********/
bool telemetry_setup();
void tx_task();
void rx_task();

#endif
