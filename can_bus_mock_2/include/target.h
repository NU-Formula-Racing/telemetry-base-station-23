/**
 * @file target.h
 * @author Derek Guo
 * @brief Specify which CAN sim program to compile
 * @version 1
 * @date 2023-04-01
 * 
 * @copyright Copyright (c) 2023
 * 
 */

#ifndef TARGET_H
#define TARGET_H

/********** DEFINES **********/
/**
 * Specify which of the device programs to compile:
 * - HIGH: ESP32 connected to CAN HI
 * - LOW:  ESP32 connected to CAN LO
 * 
 * To differentiate these macros from other library macros (FlexCAN),
 * the prefix TELEMETRY_BASE_STATION_ is added.
 * 
 * Only uncomment ONE or undef stuff may happen!!!
 */

#define CAN_BUS_HI
#define CAN_BUS_LO

#endif