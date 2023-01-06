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

#define CAN_BUS_LO 1
#define CAN_BUS_HI 2

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
