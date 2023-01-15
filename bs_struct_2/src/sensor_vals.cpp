#include "sensor_vals.h"

#ifdef TELEMETRY_BASE_STATION_TX

void get_msg_code(message_code_t* mc, flag_t* fast_flag, flag_t* slow_flag, uint8_t* cond_flag) {
  // Update message code, then reset flags
  if (*fast_flag) {
    *mc |= 0b1;
    *fast_flag = 0;
  }

  // if (*med_flag) {
  //   *mc |= 0b10;
  //   *med_flag = 0;
  // }

  if (*slow_flag) {
    *mc |= 0b100;
    *slow_flag = 0;
  }

  if (*cond_flag) {
    *mc |= ((uint16_t) *cond_flag) << 3;
    *cond_flag = 0;
  }
}

void serialize(message_code_t* mc, sensor_refs_t* sensor_refs, uint8_t* data_buf, size_t* buf_len) {
  // Reset buffer length and check if no updates need to be sent
  *buf_len = 0;
  if (!mc) {
    return;
  }

  // Pointer casting to allow for pointer arithmetic from the reference representation
  // Even though this is supposed to be a pointer to a pointer, it must be treated as a byte pointer
  // for the arithmetic to work (and for deserialization to be automated)
  uint8_t* sensor_data_ptr = (uint8_t*) sensor_refs;

  // Casting to access individual bytes from each of the pointers to the sensors
  uint8_t* sensor_data_bytes;

  // Borrowing visitor for buffer
  uint8_t* buf_ptr = data_buf;

  /* Data format */
  // Buffer data is written in the following format:
  // [ Code (Constituent data) ]

  /* Message code */
  // The code should be the first thing the deserializer checks to know what data is incoming
  *buf_ptr = *mc;
  buf_ptr += sizeof(message_code_t);

  /* Data serialization */
  // Dependent on code and the FMS-C priority
  size_t i, j;

  // Regular updating sensors
  if (mc & 0b1) {
    for (i = 0; i < NUM_FAST_SENSORS; ++i) {
      sensor_data_bytes = *((uint8_t**) sensor_data_ptr);;
      for (j = 0; j < fast_sensors_size[i]; ++j) {
        *(buf_ptr++) = *(sensor_data_ptr++);
        ++*buf_len;
      }
      sensor_data_ptr += SENSOR_PTR_LEN;
    }
  }

  if (mc & 0b10) {
    sensor_data_ptr = (uint8_t*) sensor_vals + FAST_SENSORS_LEN;
    for (i = 0; i < MED_SENSORS_LEN; ++i) {
      *(buf_ptr++) = *(sensor_data_ptr++);
      ++*buf_len;
    }
  }

  if (mc & 0b100) {
    sensor_data_ptr = (uint8_t*) sensor_vals + FAST_SENSORS_LEN /* + MED_SENSORS_LEN */;
    for (i = 0; i < SLOW_SENSORS_LEN; ++i) {
      *(buf_ptr++) = *(sensor_data_ptr++);
      ++*buf_len;
    }
  }

  // Conditional sensors
  // Handled individually
  if (mc & 0b10000) {
    sensor_data_ptr = (uint8_t*) sensor_vals + FAST_SENSORS_LEN /* + MED_SENSORS_LEN */ + SLOW_SENSORS_LEN;
    *(buf_ptr++) = *(sensor_data_ptr);
    ++*buf_len;
  }
}

#endif

#ifdef TELEMETRY_BASE_STATION_RX

void deserialize(message_code_t* mc, sensor_vals_t* sensor_vals, uint8_t* data_buf) {
  // Recasts for struct and buffer to allow for access to individual bytes
  uint8_t* sensor_data_ptr = (uint8_t*) sensor_vals;
  uint8_t* buf_ptr = data_buf;

  /* Get message code */
  *mc = *((message_code_t*) buf_ptr);
  buf_ptr += sizeof(message_code_t);

  /* Update corresponding struct data */
  size_t i;

  // Regularly updating sensors
  if (*mc & 0b1) {
    for (i = 0; i < FAST_SENSORS_LEN; ++i) {
      *(sensor_data_ptr++) = *(buf_ptr++);
    }
  }

  // if (*mc & 0b10) {
  //   sensor_data_ptr = (uint8_t*) sensor_vals + FAST_SENSORS_LEN;
  //   for (i = 0; i < MED_SENSORS_LEN; ++i) {
  //     *(sensor_data_ptr++) = *(buf_ptr++);
  //   }
  // }

  if (*mc & 0b100) {
    sensor_data_ptr = (uint8_t*) sensor_vals + FAST_SENSORS_LEN /* + MED_SENSORS_LEN */;
    for (i = 0; i < SLOW_SENSORS_LEN; ++i) {
      *(sensor_data_ptr++) = *(buf_ptr++);
    }
  }

  // Conditional sensors
  if (*mc & 0b10000) {
    sensor_data_ptr = (uint8_t*) sensor_vals + FAST_SENSORS_LEN /* + MED_SENSORS_LEN */ + SLOW_SENSORS_LEN;
    *(sensor_data_ptr) = *(buf_ptr++);
  }
}

#endif