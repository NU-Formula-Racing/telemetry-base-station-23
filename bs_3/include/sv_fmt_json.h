/**
 * @file sv_fmt_json.h
 * @author Derek Guo
 * @brief Template for parsing sensor data into JSON format
 * @version 1
 * @date 2023-02-08
 * 
 * @copyright Copyright (c) 2023
 * 
 */

#ifndef SV_FMT_JSON_H
#define SV_FMT_JSON_H

/********** INCLUDES **********/
// Target header file
#include "sensor_vals.h"

/********** FUNCTIONS **********/
/**
 * @brief Prints data in a sensor values struct in the JSON format using Arduino Serial
 * @param mc  Pointer to message code
 * @param sv  Pointer to struct containing all sensor data
 */
void sv_fmt_json(message_code_t* mc, sensor_vals_t* sv);

#endif