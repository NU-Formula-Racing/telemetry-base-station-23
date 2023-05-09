/**
 * @file telemetry.cpp
 * @author Chris Uustal, Derek Guo
 * @brief NFR Telemetry firmware TX and RX code
 * @version 2
 * @date 2022-10-22
 * 
 * @copyright Copyright (c) 2022
 * 
 */

/********** INCLUDES **********/
#include "telemetry.h"

#include "target.h"
#include "sensor_vals.h"
#include "chronoshift.h"

#ifdef TELEMETRY_BASE_STATION_TX
  // CAN library for Teensy
  #include "teensy_can.h"
#endif

#ifdef TELEMETRY_BASE_STATION_RX
  // JSON formatter for Telemetry Live
  #include "sv_fmt_json.h"
#endif

/********** LORA VARIABLES **********/
// Information related to the radio

/* RadioHead */
// Driver
RH_RF95 rf95(RFM95_CS, RFM95_INT);

// Success
bool rfm95_init_successful = true;

/* Payload information */
// Message code
message_code_t data_id = 0;

// Data buffer
uint8_t buf[RH_RF95_MAX_MESSAGE_LEN];

// Buffer length
#ifdef TELEMETRY_BASE_STATION_TX
  size_t buf_len = RH_RF95_MAX_MESSAGE_LEN;
#endif
#ifdef TELEMETRY_BASE_STATION_RX
  uint8_t buf_len = RH_RF95_MAX_MESSAGE_LEN;
#endif

/********** CAN VARIABLES **********/
// Information related to CAN interfacing
// TODO: add Inverter interface simulation support

#ifdef TELEMETRY_BASE_STATION_TX
  /* CAN Busses */
  TeensyCAN<CAN_BUS_HI> bus_hi{};
  TeensyCAN<CAN_BUS_LO> bus_lo{};

  /* Sensor Variables (info) */
  // HI to LO
  // Increasing address locations
  // Signals before messages
  // Increasing start bit

  /* HI Bus Level Sensors */

  // High Voltage BMS (values and signalling)
  CANSignal<float, 0, 12, CANTemplateConvertFloat(0.1), CANTemplateConvertFloat(0)> hv_max_discharge_current_sig;
  CANSignal<float, 12, 12, CANTemplateConvertFloat(0.1), CANTemplateConvertFloat(0)> hv_max_regen_current_sig;
  CANSignal<float, 24, 16, CANTemplateConvertFloat(0.01), CANTemplateConvertFloat(0)> hv_battery_voltage_sig;
  CANSignal<int8_t, 40, 8, CANTemplateConvertFloat(1), CANTemplateConvertFloat(-40)> hv_battery_temperature_sig;
  CANSignal<float, 48, 16, CANTemplateConvertFloat(0.01), CANTemplateConvertFloat(0), true> hv_battery_current_sig;

  CANRXMessage<5> hv_bms_soe_msg{bus_hi, 0x240, hv_max_discharge_current_sig, hv_max_regen_current_sig, hv_battery_voltage_sig, hv_battery_temperature_sig, hv_battery_current_sig};

  CANSignal<uint8_t, 40, 8, CANTemplateConvertFloat(1), CANTemplateConvertFloat(0)> hv_state_of_charge_sig;

  CANRXMessage<1> hv_bms_status_msg{bus_hi, 0x241, hv_state_of_charge_sig};

  // Throttle
  CANSignal<int8_t, 0, 8, CANTemplateConvertFloat(1), CANTemplateConvertFloat(0), true> accel_percentage_sig;
  CANSignal<uint8_t, 8, 8, CANTemplateConvertFloat(1), CANTemplateConvertFloat(0)> brake_percentage_sig;

  CANRXMessage<2> throttle_values_msg{bus_hi, 0x300, accel_percentage_sig, brake_percentage_sig};

  CANSignal<uint8_t /* enum */, 0, 8, CANTemplateConvertFloat(1), CANTemplateConvertFloat(0)> tractile_system_status_sig;

  CANRXMessage<1> throttle_status_msg{bus_hi, 0x301, tractile_system_status_sig};

  // Temperature board
  CANSignal<float, 16, 16, CANTemplateConvertFloat(0.1), CANTemplateConvertFloat(-40)> coolant_temperature_sig;
  CANSignal<float, 32, 16, CANTemplateConvertFloat(0.1), CANTemplateConvertFloat(-40)> ambient_temperature_sig;

  CANRXMessage<2> ptrain_temp_msg{bus_hi, 0x420, coolant_temperature_sig, ambient_temperature_sig};

  CANSignal<float, 0, 16, CANTemplateConvertFloat(0.01), CANTemplateConvertFloat(0)> coolant_flow_sig;

  CANRXMessage<1> coolant_msg{bus_hi, 0x421, coolant_flow_sig};

  // Inverter (TODO)
  float motor_temperature = 0.0f;
  float inverter_temperature = 0.0f;
  float rpm = 0.0f;

  float* motor_temperature_ref = &motor_temperature;
  float* inverter_temperature_ref = &inverter_temperature;
  float* rpm_ref = &rpm;

  /* LO Bus Level Sensors */

  // Wheels (speed and brake temps)
  CANSignal<float, 0, 16, CANTemplateConvertFloat(0.1), CANTemplateConvertFloat(0)> fl_wheel_speed_sig;
  CANSignal<float, 16, 16, CANTemplateConvertFloat(0.1), CANTemplateConvertFloat(-40)> fl_brake_temperature_sig;

  CANRXMessage<2> fl_wheel_msg{bus_lo, 0x400, fl_wheel_speed_sig, fl_brake_temperature_sig};

  CANSignal<float, 0, 16, CANTemplateConvertFloat(0.1), CANTemplateConvertFloat(0)> fr_wheel_speed_sig;
  CANSignal<float, 16, 16, CANTemplateConvertFloat(0.1), CANTemplateConvertFloat(-40)> fr_brake_temperature_sig;

  CANRXMessage<2> fr_wheel_msg{bus_lo, 0x401, fr_wheel_speed_sig, fr_brake_temperature_sig};

  CANSignal<float, 0, 16, CANTemplateConvertFloat(0.1), CANTemplateConvertFloat(0)> bl_wheel_speed_sig;
  CANSignal<float, 16, 16, CANTemplateConvertFloat(0.1), CANTemplateConvertFloat(-40)> bl_brake_temperature_sig;

  CANRXMessage<2> bl_wheel_msg{bus_lo, 0x402, bl_wheel_speed_sig, bl_brake_temperature_sig};

  CANSignal<float, 0, 16, CANTemplateConvertFloat(0.1), CANTemplateConvertFloat(0)> br_wheel_speed_sig;
  CANSignal<float, 16, 16, CANTemplateConvertFloat(0.1), CANTemplateConvertFloat(-40)> br_brake_temperature_sig;

  CANRXMessage<2> br_wheel_msg{bus_lo, 0x403, br_wheel_speed_sig, br_brake_temperature_sig};

  // Brake Pressures
  CANSignal<uint16_t, 0, 16, CANTemplateConvertFloat(1), CANTemplateConvertFloat(0)> front_brake_pressure_sig;
  CANSignal<uint16_t, 16, 16, CANTemplateConvertFloat(1), CANTemplateConvertFloat(0)> rear_brake_pressure_sig;

  CANRXMessage<2> brake_pressure_msg{bus_lo, 0x410, front_brake_pressure_sig, rear_brake_pressure_sig};

  // Motion Board
  CANSignal<int32_t, 0, 32, CANTemplateConvertFloat(1), CANTemplateConvertFloat(0), true> latitude_sig;
  CANSignal<int32_t, 32, 32, CANTemplateConvertFloat(1), CANTemplateConvertFloat(0), true> longitude_sig;

  CANRXMessage<2> motion_gps_msg{bus_lo, 0x430, latitude_sig, longitude_sig};

  CANSignal<float, 0, 16, CANTemplateConvertFloat(0.0005), CANTemplateConvertFloat(0), true> accel_x_sig;
  CANSignal<float, 16, 16, CANTemplateConvertFloat(0.0005), CANTemplateConvertFloat(0), true> accel_y_sig;
  CANSignal<float, 32, 16, CANTemplateConvertFloat(0.0005), CANTemplateConvertFloat(0), true> accel_z_sig;

  CANRXMessage<3> motion_accelerometer_msg{bus_lo, 0x431, accel_x_sig, accel_y_sig, accel_z_sig};

  CANSignal<float, 0, 16, CANTemplateConvertFloat(0.0005), CANTemplateConvertFloat(0), true> gyro_x_sig;
  CANSignal<float, 16, 16, CANTemplateConvertFloat(0.0005), CANTemplateConvertFloat(0), true> gyro_y_sig;
  CANSignal<float, 32, 16, CANTemplateConvertFloat(0.0005), CANTemplateConvertFloat(0), true> gyro_z_sig;

  CANRXMessage<3> motion_gyroscope_msg{bus_lo, 0x432, gyro_x_sig, gyro_y_sig, gyro_z_sig};

  // RTC
  CANSignal<uint32_t, 0, 32, CANTemplateConvertFloat(1), CANTemplateConvertFloat(0)> rtc_sig;

  CANRXMessage<1> rtc_msg{bus_lo, 0x440, rtc_sig};
#endif

/********** FW VARIABLES **********/
// Temp vars or intermediates specific to the TX/RX firmware

/* Storage for sensor data */
#ifdef TELEMETRY_BASE_STATION_TX
  // Sensor value reference struct
  sensor_refs_t sensor_refs;
#endif

#ifdef TELEMETRY_BASE_STATION_RX
  // Sensor value data struct
  sensor_vals_t sensor_vals;

  // Debug var
  uint8_t* buf_ptr;
  uint8_t* sensor_vals_ptr;
#endif

#ifdef TELEMETRY_BASE_STATION_TX
  /* Flags */
  flag_t slow_flag = 0;
  // flag_t med_flag = 0;
  flag_t fast_flag = 0;
  uint32_t cond_flag = 0;
#endif

/********** FUNCTIONS **********/

/**
 * @brief Runs all setup functions for the RFM95 module 
 * @return true if setup was successful
 * @return false if setup was unsuccessful
 */
bool telemetry_setup() {
  pinMode(RFM95_RST, OUTPUT);
  digitalWrite(RFM95_RST, HIGH);

  // Manual reset
  digitalWrite(RFM95_RST, LOW);
  delay(10);
  digitalWrite(RFM95_RST, HIGH);
  delay(10);

  // Turn on linear regulator
  pinMode(TEENSY40_CE, OUTPUT);
  digitalWrite(TEENSY40_CE, HIGH);

  // Set up RadioHead
  if (rf95.init() == true) {
    // Defaults after init are 434.0MHz, modulation GFSK_Rb250Fd250, +13dbM
    if (rf95.setFrequency(RF95_FREQ) == true) {
      // Defaults after init are 434.0MHz, 13dBm, Bw = 125 kHz, Cr = 4/5, Sf = 128chips/symbol, CRC on

      // The default transmitter power is 13dBm, using PA_BOOST.
      // If you are using RFM95/96/97/98 modules which uses the PA_BOOST transmitter pin, then 
      // you can set transmitter powers from 5 to 23 dBm:
      rf95.setTxPower(23, false);
    } else {
      Serial.println("setFrequency failed");
      rfm95_init_successful = false;
    }
  } else {
    Serial.println("LoRa radio init failed");
    rfm95_init_successful = false;
  }

  #ifdef TELEMETRY_BASE_STATION_TX
    // Initialize CAN bus
    bus_hi.RegisterRXMessage(hv_bms_soe_msg);
    bus_hi.RegisterRXMessage(hv_bms_status_msg);
    bus_hi.RegisterRXMessage(throttle_values_msg);
    bus_hi.RegisterRXMessage(throttle_status_msg);
    bus_hi.RegisterRXMessage(ptrain_temp_msg);
    bus_hi.RegisterRXMessage(coolant_msg);
    bus_lo.RegisterRXMessage(fl_wheel_msg);
    bus_lo.RegisterRXMessage(fr_wheel_msg);
    bus_lo.RegisterRXMessage(bl_wheel_msg);
    bus_lo.RegisterRXMessage(br_wheel_msg);
    bus_lo.RegisterRXMessage(brake_pressure_msg);
    bus_lo.RegisterRXMessage(motion_gps_msg);
    bus_lo.RegisterRXMessage(motion_accelerometer_msg);
    bus_lo.RegisterRXMessage(motion_gyroscope_msg);
    bus_lo.RegisterRXMessage(rtc_msg);

    bus_hi.Initialize(ICAN::BaudRate::kBaud1M);
    bus_lo.Initialize(ICAN::BaudRate::kBaud1M);

    // Get and store sensor references
    sensor_refs = {
      .fast = {
        .fl_wheel_speed = &(fl_wheel_speed_sig.value_ref()),
        .fr_wheel_speed = &(fr_wheel_speed_sig.value_ref()),
        .bl_wheel_speed = &(bl_wheel_speed_sig.value_ref()),
        .br_wheel_speed = &(br_wheel_speed_sig.value_ref()),
        .hv_battery_voltage = &(hv_battery_voltage_sig.value_ref()),
        .motor_temperature = motor_temperature_ref, // &(motor_temperature_sig.value_ref()),
        .accel_x = &(accel_x_sig.value_ref()),
        .accel_y = &(accel_y_sig.value_ref()),
        .accel_z = &(accel_z_sig.value_ref()),
        .gyro_x = &(gyro_x_sig.value_ref()),
        .gyro_y = &(gyro_y_sig.value_ref()),
        .gyro_z = &(gyro_z_sig.value_ref()),
        .latitude = &(latitude_sig.value_ref()),
        .longitude = &(longitude_sig.value_ref()),
        .rpm = rpm_ref,// &(rpm_sig.value_ref()),
        .hv_battery_current = &(hv_battery_current_sig.value_ref()),
        .hv_max_discharge_current = &(hv_max_discharge_current_sig.value_ref()),
        .hv_max_regen_current = &(hv_max_regen_current_sig.value_ref()),
        .rtc = &(rtc_sig.value_ref()),
        .front_brake_pressure = &(front_brake_pressure_sig.value_ref()),
        .rear_brake_pressure = &(rear_brake_pressure_sig.value_ref()),
        .hv_battery_temperature = &(hv_battery_temperature_sig.value_ref()),
        .tractile_system_status = &(tractile_system_status_sig.value_ref()),
        .accel_percentage = &(accel_percentage_sig.value_ref()),
        .brake_percentage = &(brake_percentage_sig.value_ref())
      },
      .slow = {
        .coolant_temperature = &(coolant_temperature_sig.value_ref()),
        .fl_brake_temperature = &(fl_brake_temperature_sig.value_ref()),
        .fr_brake_temperature = &(fr_brake_temperature_sig.value_ref()),
        .bl_brake_temperature = &(bl_brake_temperature_sig.value_ref()),
        .br_brake_temperature = &(br_brake_temperature_sig.value_ref()),
        .ambient_temperature = &(ambient_temperature_sig.value_ref()),
        .inverter_temperature = inverter_temperature_ref, //&(inverter_temperature_sig.value_ref()),
        .coolant_flow = &(coolant_flow_sig.value_ref()),
        .hv_state_of_charge = &(hv_state_of_charge_sig.value_ref())
      }
    };
  #endif

  return rfm95_init_successful;
}

/*** TRANSCEIVER CODE ***/

/**
 * @brief Update sensors deemed "fast"
 * 
 */
void tx_tick_fast() {
  #ifdef TELEMETRY_BASE_STATION_TX
    bus_hi.Tick();
    bus_lo.Tick();
    fast_flag = 1;
  #endif
}

/**
 * @brief Update sensors deemed "slow"
 * 
 */
void tx_tick_slow() {
  #ifdef TELEMETRY_BASE_STATION_TX
    // Need a concrete way to determine if new slow data has been sent; currently unknown
    slow_flag = 1;
  #endif
}

/**
 * @brief Transmit data
 * 
 */
void tx_send() {
  if (rfm95_init_successful == true) {
    #ifdef TELEMETRY_BASE_STATION_TX
      // // From the flags, transcribe them into a singular code, then reset flags
      // get_msg_code(&data_id, &fast_flag, &slow_flag, &cond_flag);
      
      // // // Serialize the data
      // serialize(&data_id, &sensor_refs, buf, &buf_len);

      // // Send data and verify completion
      // // delay(10);
      // rf95.send(buf, SENSOR_VALS_LEN); // buf_len);
      // // delay(10);
      // rf95.waitPacketSent();
      
      // Test: accuracy
      Serial.print("accel: (");
      Serial.print("x: "); Serial.print(accel_x_sig.value_ref());
      Serial.print(", y: "); Serial.print(accel_y_sig.value_ref());
      Serial.print(", z: "); Serial.print(accel_z_sig.value_ref());
      Serial.print("), gyro: (");
      Serial.print("x: "); Serial.print(gyro_x_sig.value_ref());
      Serial.print(", y: "); Serial.print(gyro_y_sig.value_ref());
      Serial.print(", z: "); Serial.print(gyro_z_sig.value_ref());
      Serial.println(")");
    #endif
  }
}

/*** TRANSCEIVER CODE ***/

/**
 * @brief 
 * 
 */
void rx_task() {
  if (rf95.available() && (rfm95_init_successful == true)) {
    #ifdef TELEMETRY_BASE_STATION_RX
      // Check if a message has been received, and retrieve its data and length
      if (rf95.recv(buf, &buf_len)) {
        // // Receive successful
        // RH_RF95::printBuffer("Received ", buf, buf_len);
        // Serial.print("Got: "); Serial.println((char*) buf);

        // sensor_vals_ptr = (uint8_t*) &(sensor_vals.fast.rtc);
        // buf_ptr = buf;
        // *(sensor_vals_ptr++) = *(buf_ptr++);
        // *(sensor_vals_ptr++) = *(buf_ptr++);
        // *(sensor_vals_ptr++) = *(buf_ptr++);
        // *sensor_vals_ptr = *buf_ptr;
        // Serial.println(sensor_vals.fast.rtc);

        // Parse data
        deserialize(&data_id, &sensor_vals, buf);

        // Format data as JSON and print it
        sv_fmt_json(&data_id, &sensor_vals);

      } else {
        Serial.println("Receive failed");
      }
    #endif
  }
}