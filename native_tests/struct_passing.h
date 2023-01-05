/**
 * @file struct_passing.h
 * @author Derek Guo
 * @brief Dynamic storage struct and ser/des functions for its data transfer
 * @version 1
 * @date 2022-12-15
 * 
 * @copyright Copyright (c) 2022
 * 
 */

/********** INCLUDES **********/
#include <stdint.h>
#include <stdlib.h>

// Assume compilation on 32/64 bit platform, so float is 4 bytes
// Fast = fastest updating
// Med = moderate fast updating
// Slow = slowest updating
// Cond = arbitrary update
// None of the rates are assumed to be multiples, but might

/********** TYPE DEFINITIONS **********/
#pragma pack(1)
typedef struct FAKE_SENSORS {
  float fast_fl;
  uint16_t fast_ui;
  uint8_t fast_sig;
  float med_fl;
  uint16_t med_ui;
  uint8_t med_sig;
  float slow_fl;
  uint16_t slow_ui;
  uint8_t slow_sig;
  uint8_t cond_sig;
} fake_sensors_t;

// Macros for data properties in test program
#define FAKE_SENSORS_LEN sizeof(fake_sensors_t)
#define FMT_FAKE_SENSORS(id, s) printf("%ld:\t{fast: %.1f, %u, %u; med: %.1f, %u, %u; slow: %.1f, %u, %u; cond: %u}\n", id, s.fast_fl, s.fast_ui, s.fast_sig, s.med_fl, s.med_ui, s.med_sig, s.slow_fl, s.slow_ui, s.slow_sig, s.cond_sig)

// ID that describes what data the incoming message packet contains,
// formatted in binary as
// `0bAAAABBBB`
// A - any conditionally varying data
// B - regularly updating data
//   - 0SMF
//     - S = slow
//     - M = med
//     - F = fast
typedef uint16_t message_id_t;

/********** FUNCTIONS **********/
/**
 * @brief Format data from a struct to fit a message packet buffer.
 * @param msg_id        Type/amount of data to be sent, dictating which data should be put in and how
 * @param sensor_struct Pointer to struct containing all sensor data
 * @param data_buf      Message buffer to be sent, containing the message type and payload
 * @param buf_len       Total amount of data written to the buffer, needed to optimize packet messaging
 */
void serialize(message_id_t msg_id, fake_sensors_t* sensor_struct, uint8_t* data_buf, size_t* buf_len);

/**
 * @brief Reformat buffered data to update the values within a struct.
 * @param msg_id        Type/amount of data to be sent, dictating where data should be placed
 * @param sensor_struct Pointer to struct containing all sensor data
 * @param data_buf      Message buffer received, containing the message type and payload
 */
void deserialize(message_id_t* msg_id, fake_sensors_t* sensor_struct, uint8_t* data_buf);