/**
 * @file sensor_vals.h
 * @author Derek Guo
 * @brief Dynamic storage struct and ser/des methods for sensor vals
 * @version 1
 * @date 2023-01-06
 * 
 * @copyright Copyright (c) 2023
 * 
 */

/********** CONVENTIONS **********/
/* KEYWORDS */
// Update   = modified from ticking CAN bus, reading it to signal register
// Transmit = sent over from Base Station RX to TX (through LoRa)

// Fast = fastest updating, almost all transmissions will contain these
// Med  = medium / moderate updating, used depending on scale
// Slow = slowest updating
// Cond = arbitrary / transient update, either rare to occur or manually controlled

// Flag = firmware control specifying that certain vals are updated, need to be transmitted

// Please use the type aliases and size macros, not the `structs`!

#ifndef SENSOR_VALS_H
#define SENSOR_VALS_H

/********** INCLUDES **********/
#include "telemetry.h"

/********** STRUCTS **********/

/* Fast updating sensors */
#pragma pack(1)
struct FAST_SENSORS {
  float fl_wheel_speed;
  float fl_brake_temperature;
  float fr_wheel_speed;
  float fr_brake_temperature;
  float bl_wheel_speed;
  float bl_brake_temperature;
  float br_wheel_speed;
  float br_brake_temperature;
  uint16_t front_brake_pressure;
  uint16_t rear_brake_pressure;
};
typedef struct FAST_SENSORS fast_sensors_t;

/* Slow updating sensors */
#pragma pack(1)
struct SLOW_SENSORS {
  uint16_t fake_value;
};
typedef struct SLOW_SENSORS slow_sensors_t;

/* Composite sensor struct */
#pragma pack(1)
struct SENSOR_VALS {
  fast_sensors_t fast;
  slow_sensors_t slow;
  uint16_t packetnum;
};
typedef struct SENSOR_VALS sensor_vals_t;

/********** DEFINES **********/
/* Byte size */
#define FAST_SENSORS_LEN sizeof(fast_sensors_t)
#define SLOW_SENSORS_LEN sizeof(slow_sensors_t)
#define SENSOR_VALS_LEN sizeof(sensor_vals_t)

/********** TYPE ALIASES **********/
/* Update flag signal */
// Should be efficient to store + alter, only needs 1 bit
typedef uint32_t flag_t;

// Cond signalling will be done using unsigneds for now

/* Message code */
// Describes what data the incoming message packet contains
// and its format, itsef formatted in binary as
// `0bAAAAABBB`
// A - controls for any Cond data
// B - regularly updating data, its bits interpreted in order:
//   - `SMF`
//     - S = Slow
//     - M = Med
//     - F = Fast
typedef uint32_t message_code_t;

/********** METHODS **********/
/**
 * @brief Format data from a struct to fit a message packet buffer, then resets flags.
 * @param mc            Message code, type/amount of data to be sent
 * @param fast_flag     Flag for fast sensors
 * @param slow_flag     Flag for slow sensors
 * @param cond_flag     Aggregate signal for any number of cond sensors
 */
void get_msg_code(message_code_t* mc, flag_t* fast_flag, flag_t* slow_flag, uint32_t* cond_flag);

/**
 * @brief Format data from a struct to fit a message packet buffer.
 * @param mc            Type/amount of data to be sent, dictating which data should be put in and how
 * @param sensor_vals   Pointer to struct containing all sensor data
 * @param data_buf      Message buffer to be sent, containing the message type and payload
 * @param buf_len       Total amount of data written to the buffer, needed to optimize packet messaging
 */
void serialize(message_code_t mc, sensor_vals_t* sensor_vals, uint8_t* data_buf, size_t* buf_len);

/**
 * @brief Reformat buffered data to update the values within a struct.
 * @param mc            Type/amount of data to be sent, dictating where data should be placed
 * @param sensor_vals   Pointer to struct containing all sensor data
 * @param data_buf      Message buffer received, containing the message type and payload
 */
void deserialize(message_code_t* mc, sensor_vals_t* sensor_vals, uint8_t* data_buf);

#endif