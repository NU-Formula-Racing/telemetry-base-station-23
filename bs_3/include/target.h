/**
 * @file target.h
 * @author Derek Guo
 * @brief Specify which program to compile, which applies to multiple files
 * @version 1
 * @date 2022-10-22
 * 
 * @copyright Copyright (c) 2022
 * 
 */

#ifndef TARGET_H
#define TARGET_H

/********** DEFINES **********/
/**
 * Specify which of the device programs to compile:
 * - TX: receive from CAN, send to RX
 * - RX: receive from RX, sent through USB
 * 
 * To differentiate these macros from other library macros (FlexCAN),
 * the prefix TELEMETRY_BASE_STATION_ is added.
 * 
 * Only uncomment ONE or undef stuff may happen!!!
 */

#define TELEMETRY_BASE_STATION_TX
// #define TELEMETRY_BASE_STATION_RX

#endif