/**
 * @file telemetry.h
 * @author Chris Uustal, Derek Guo
 * @brief Header file for telemetry firmware server (TX) and client (RX) functions
 * @version 2
 * @date 2023-04-29
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
/* RadioHead */
#define RFM95_CS 10
#define RFM95_RST 2
#define RFM95_INT 3
#define RF95_FREQ 915.0

/* CAN Priority */
#define CAN_BUS_LO 1
#define CAN_BUS_HI 2

/* Linear regulator */
#define TEENSY40_CE 4

/********** VARIABLES **********/
// Singleton instance of the radio driver
extern RH_RF95 rf95;

extern bool rfm95_init_successful;

extern uint16_t packetnum;  // packet counter, we increment per xmission

/********** PUBLIC FUNCTION PROTOTYPES **********/

/* Setup (for both boards) */
bool telemetry_setup();

/* Transceiver */
void tx_tick_fast();
void tx_tick_slow();
void tx_send();

void tx_task();

/* Receiver */
void rx_task();

#endif
