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
#include "target.h"

/********** STRUCTS **********/

// All structs have a value representation (which will be used for direct ser/des),
// a reference representation to avoid redundant copying in TX,
// and a size chart used in conjunction with the reference representation.
// Both need to be maintained as sensor demands change.

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
  uint16_t packetnum;
};
typedef struct FAST_SENSORS fast_sensors_t;

#ifdef TELEMETRY_BASE_STATION_TX

struct FAST_SENSORS_REF {
  float* fl_wheel_speed;
  float* fl_brake_temperature;
  float* fr_wheel_speed;
  float* fr_brake_temperature;
  float* bl_wheel_speed;
  float* bl_brake_temperature;
  float* br_wheel_speed;
  float* br_brake_temperature;
  uint16_t* front_brake_pressure;
  uint16_t* rear_brake_pressure;
  uint16_t* packetnum;
};
typedef struct FAST_SENSORS_REF fast_sensor_refs_t;

#define NUM_FAST_SENSORS 11
const size_t fast_sensors_size[NUM_FAST_SENSORS] = {
  sizeof(float),
  sizeof(float),
  sizeof(float),
  sizeof(float),
  sizeof(float),
  sizeof(float),
  sizeof(float),
  sizeof(float),
  sizeof(uint16_t),
  sizeof(uint16_t),
  sizeof(uint16_t)
};

#endif

/* Med updating sensors */
#pragma pack(1)
struct MED_SENSORS {
  uint16_t fake_value;
};
typedef struct MED_SENSORS med_sensors_t; // Currently omitted for simplicity

#ifdef TELEMETRY_BASE_STATION_TX

struct MED_SENSORS_REF {
  uint16_t* fake_value;
};
typedef struct MED_SENSORS_REF med_sensor_refs_t;

#define NUM_MED_SENSORS 1
const size_t med_sensors_size[NUM_MED_SENSORS] = {
  sizeof(uint16_t)
};

#endif

/* Slow updating sensors */
#pragma pack(1)
struct SLOW_SENSORS {
  uint16_t fake_value;
};
typedef struct SLOW_SENSORS slow_sensors_t;

#ifdef TELEMETRY_BASE_STATION_TX

struct SLOW_SENSORS_REF {
  uint16_t* fake_value;
};
typedef struct SLOW_SENSORS_REF slow_sensor_refs_t;

#define NUM_SLOW_SENSORS 1
const size_t slow_sensors_size[NUM_SLOW_SENSORS] = {
  sizeof(uint16_t)
};

#endif

/* Composite sensor struct */
#pragma pack(1)
struct SENSOR_VALS {
  fast_sensors_t fast;
  // med_sensors_t med;
  slow_sensors_t slow;
  uint8_t control;
};
typedef struct SENSOR_VALS sensor_vals_t;

#ifdef TELEMETRY_BASE_STATION_TX

// All regularly updated sensors use pointers to facilitate automation of serialization.
// All conditional values use references for safety, as each are handled individually.
struct SENSOR_REFERENCES {
  fast_sensor_refs_t fast;
  // med_sensor_refs_t med;
  slow_sensor_refs_t slow;
  uint8_t* control;
};
typedef struct SENSOR_REFERENCES sensor_refs_t;

#endif

/********** DEFINES **********/
/* Byte size of sensor vals */
#define FAST_SENSORS_LEN sizeof(fast_sensors_t)
#define MED_SENSORS_LEN sizeof(med_sensors_t)
#define SLOW_SENSORS_LEN sizeof(slow_sensors_t)
#define SENSOR_VALS_LEN sizeof(sensor_vals_t)

/* Size of pointer representation */
#ifdef TELEMETRY_BASE_STATION_TX
  #define SENSOR_PTR_LEN sizeof(float*)
#endif

/********** TYPE ALIASES **********/
/* Update flag signal */
// Should be efficient to store + alter, only needs 1 bit
typedef uint16_t flag_t;

// Cond signalling will be done using unsigned bytes for now

/* Message code */
// Describes what data the incoming message packet contains
// and its format, itsef formatted in binary as
// `0bAAA...AABBB`
// A - controls for any Cond data
// B - regularly updating data, its bits interpreted in order:
//   - `SMF`
//     - S = Slow
//     - M = Med
//     - F = Fast
typedef uint16_t message_code_t;

/********** METHODS **********/

/* Transceiver (car board) */
#ifdef TELEMETRY_BASE_STATION_TX

/**
 * @brief Format data from a struct to fit a message packet buffer, then resets flags.
 * @param mc            Message code, type/amount of data to be sent
 * @param fast_flag     Flag for fast sensors
 * @param fast_flag     Flag for med sensors (omitted)
 * @param slow_flag     Flag for slow sensors
 * @param cond_flag     Aggregate signal for any number of cond sensors
 */
void get_msg_code(message_code_t* mc, flag_t* fast_flag, /* flag_t* med_flag, */ flag_t* slow_flag, uint32_t* cond_flag);

/**
 * @brief Format data from a struct to fit a message packet buffer.
 * @param mc            Type/amount of data to be sent, dictating which data should be put in and how
 * @param sensor_vals   Pointer to struct containing all sensor data
 * @param data_buf      Message buffer to be sent, containing the message type and payload
 * @param buf_len       Total amount of data written to the buffer, needed to optimize packet messaging
 */
void serialize(message_code_t* mc, sensor_refs_t* sensor_refs, uint8_t* data_buf, size_t* buf_len);

#endif

/* Receiver (base board) */
/**
 * @brief Reformat buffered data to update the values within a struct.
 * @param mc            Type/amount of data to be sent, dictating where data should be placed
 * @param sensor_vals   Pointer to struct containing all sensor data
 * @param data_buf      Message buffer received, containing the message type and payload
 */
void deserialize(message_code_t* mc, sensor_vals_t* sensor_vals, uint8_t* data_buf);

#endif