/**
 * @file struct_passing.c
 * @author Derek Guo
 * @brief Program to simulate ser/des with multiple update times
 * @version 1
 * @date 2022-12-15
 * 
 * @copyright Copyright (c) 2022
 * 
 */

/********** INCLUDES **********/
#include "struct_passing.h"
#include <stdio.h>

/********** MACROS **********/

// Total number of "ticks" to be simulated
#define RUN_TIME 100

// Update times
// SMF times are rates and repeat periodically
#define FAST_UPDATE 3
#define MED_UPDATE 7
#define SLOW_UPDATE 19

// Conditional times are transient and only appear once
#define COND_UPDATE 57

int main(int argc, char* argv[]) {
  /// TX data
  // Some values are zero-init, others aren't to emulate unpredictability of real values
  fake_sensors_t tx_struct;
  tx_struct.fast_fl = 0.0;
  tx_struct.fast_ui = 0;
  tx_struct.med_fl = 0.0;
  tx_struct.med_ui = 0;
  tx_struct.slow_fl = 0.0;
  tx_struct.slow_ui = 0;

  /// RX data
  // All members are initialized to values unlikely to be present in initial TX data
  // to confirm whether or not data was properly transfered
  fake_sensors_t rx_struct = {
    .fast_fl = 1.0,
    .fast_ui = 10,
    .fast_sig = 16,
    .med_fl = 1.0,
    .med_ui = 10,
    .med_sig = 16,
    .slow_fl = 1.0,
    .slow_ui = 16,
    .slow_sig = 10.0,
    .cond_sig = 16
  };

  /// Serialized data buffer
  // Since the "transfer" takes place in one program without comm channel,
  // only a common buffer will be used.
  uint8_t data_buf[252];
  size_t buf_len;

  /// Message ID
  message_id_t mid;

  /// Iteration
  // For each tick, the mid is determined, TX data is updated
  // then the data is serialized and subsequently deserialized to RX;
  // new updates are displayed to verify that updates occurred as expected.
  for (size_t i = 0; i < RUN_TIME; ++i) {
    /// Update mid
    mid = 0;

    // The ID is determined and processed with the following priorities:
    // Fast -> Med -> Slow -> Cond
    // If multiple conditions are satisfied, the order of formatting remains the same.

    // Both the mid and the data values (except at init) are updated in the following set of conditionals.
    if (i % FAST_UPDATE == 0) {
      mid |= 0b1;
      if (i > 0) {
        tx_struct.fast_fl += 0.1;
        ++tx_struct.fast_ui;
        ++tx_struct.fast_sig;
      }
    }
    if (i % MED_UPDATE == 0) {
      mid |= 0b10;
      if (i > 0) {
        tx_struct.med_fl += 0.1;
        ++tx_struct.med_ui;
        ++tx_struct.med_sig;
      }
    }
    if (i % SLOW_UPDATE == 0) {
      mid |= 0b100;
      if (i > 0) {
        tx_struct.slow_fl += 0.1;
        ++tx_struct.slow_ui;
        ++tx_struct.slow_sig;
      }
    }
    if (i == COND_UPDATE) {
      mid |= 0b10000;
      tx_struct.cond_sig = 1;
    }

    /// Ser/Des 
    serialize(mid, &tx_struct, data_buf, &buf_len);
    if (mid != 0) { // New data incoming
      deserialize(&mid, &rx_struct, data_buf);

      // Display data if update has occurred
      FMT_FAKE_SENSORS(i, rx_struct);
    }
  }

  return 0;
}

void serialize(message_id_t msg_id, fake_sensors_t* sensor_struct, uint8_t* data_buf, size_t* buf_len) {
  // Reset buffer length and check if no updates need to be sent
  *buf_len = 0;
  if (msg_id == 0) {
    return;
  }

  // Pointer abuse to allow for raw byte data to be accessed
  uint8_t* sensor_data_ptr = (uint8_t*) sensor_struct;
  uint8_t* buf_ptr = data_buf;

  // First add int message ID
  // This should be the first thing the deserializer checks to know what data is incoming
  *buf_ptr = msg_id;
  buf_ptr += sizeof(message_id_t);

  // Serialize in FMS priority
  size_t i;
  if (msg_id & 0b1) {
    for (i = 0; i < 7; ++i) {
      *(buf_ptr++) = *(sensor_data_ptr++);
      ++*buf_len;
    }
  }
  if (msg_id & 0b10) {
    sensor_data_ptr = (uint8_t*) sensor_struct + 7;
    for (i = 0; i < 7; ++i) {
      *(buf_ptr++) = *(sensor_data_ptr++);
      ++*buf_len;
    }
  }
  if (msg_id & 0b100) {
    sensor_data_ptr = (uint8_t*) sensor_struct + 14;
    for (i = 0; i < 7; ++i) {
      *(buf_ptr++) = *(sensor_data_ptr++);
      ++*buf_len;
    }
  }
  if (msg_id & 0b10000) {
    sensor_data_ptr = (uint8_t*) sensor_struct + 21;
    *(buf_ptr++) = *(sensor_data_ptr);
    ++*buf_len;
  }
}

void deserialize(message_id_t* msg_id, fake_sensors_t* sensor_struct, uint8_t* data_buf) {
  // if (msg_id == 0) {
  //   return;
  // }
s
  // Raw data alias for struct and buffer
  uint8_t* sensor_data_ptr = (uint8_t*) sensor_struct;
  uint8_t* buf_ptr = data_buf;

  // Get message ID
  *msg_id = *((message_id_t*) buf_ptr);
  buf_ptr += sizeof(message_id_t);

  // Based on the ID, update corresponding data in struct
  size_t i;
  if (*msg_id & 0b1) {
    for (i = 0; i < 7; ++i) {
      *(sensor_data_ptr++) = *(buf_ptr++);
    }
  }
  if (*msg_id & 0b10) {
    sensor_data_ptr = (uint8_t*) sensor_struct + 7;
    for (i = 0; i < 7; ++i) {
      *(sensor_data_ptr++) = *(buf_ptr++);
    }
  }
  if (*msg_id & 0b100) {
    sensor_data_ptr = (uint8_t*) sensor_struct + 14;
    for (i = 0; i < 7; ++i) {
      *(sensor_data_ptr++) = *(buf_ptr++);
    }
  }
  if (*msg_id & 0b10000) {
    sensor_data_ptr = (uint8_t*) sensor_struct + 21;
    *(sensor_data_ptr) = *(buf_ptr++);
  }
}