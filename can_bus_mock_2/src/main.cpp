/**
 * @file telemetry.cpp
 * @author Derek Guo, Joshua
 * @brief NFR Telemetry 2-bus CAN simulator
 * @version 1
 * @date 2023-03
 * 
 * @copyright Copyright (c) 2022
 * 
 */

/********** INCLUDES **********/
#include <Arduino.h>
#include "virtualTimer.h"

#include "target.h"

#if defined(ARDUINO_TEENSY40) || defined(ARDUINO_TEENSY41)
  #include "teensy_can.h"
  typedef TeensyCAN<1> can_bus_dev_t;
#endif

#ifdef ARDUINO_ARCH_ESP32
  #include "esp_can.h"
  typedef ESPCAN can_bus_dev_t;
#endif

/********** DEFINES **********/
#define T_SEC 1000 // std::chrono::milliseconds{1000}
#define T_DS  100 // std::chrono::milliseconds{100}
#define T_CS  10 // std::chrono::milliseconds{10}
#define T_MS  1 // std::chrono::milliseconds{1}

/********** MACRO FUNCTIONS **********/
/**
 * @brief Quantized saw wave, incrementing by 1 every update and looping within bounds
 * Contract: bounds are not out-of-bounds for corresponding CAN sig
 * Contract: upper bound is not adjacent to overflow bound
 * Contract: arguments are integral
 * 
 * @param X   variable following sim, to be expanded later
 * @param L   lower bound, inclusive
 * @param H   upper bound, inclusive (reset to L once quantity exceeded)
 */
#define Q_SAW(X, L, H)\
  X = (X == H) ? L : (X + 1);

/**
 * @brief Continuous saw wave, adding given amount every loop within bounds
 * Contract: bounds are not out-of-bounds for corresponding CAN sig
 * Contract: all arguments are floating-point (no automatic cast)
 * 
 * @param X   variable following sim, to be expanded later
 * @param C   temp variable with primitive type float, reused for efficiency
 * @param A   slope over timestep of loop, either sign
 * @param L   lower bound
 * @param H   upper bound (subtract (H - L) once quantity exceeded)
 */
#define C_SAW(X, C, A, L, H) {\
  C = X + A;\
  if (C > H) {\
    C -= H;\
    C += L;\
  }\
  X = C;\
}

/**
 * @brief Integral random process, with unity in/decrement or hold per update
 * Contract: bounds are not out-of-bounds for corresponding CAN sig
 * Contract: upper bound is not close to overflow bound
 * Contract: all arguments are integral
 * 
 * @param X   variable following sim, to be expanded later
 * @param L   lower bound, inclusive
 * @param H   upper bound, inclusive
 */
#define USLINT(X, L, H) {\
  if (X == L)\
    X = X + random(2);\
  else if (X == H)\
    X = X - random(2);\
  else\
    X = X + random(3) - 1;\
}

/**
 * @brief Integral random process, with in/decrement or hold per update, clipping at bounds
 * Contract: bounds are not out-of-bounds for corresponding CAN sig
 * Contract: all arguments are floating-point (no automatic cast)
 * 
 * @param X   variable following sim, to be expanded later
 * @param C   temp variable with primitive type float, reused for efficiency
 * @param A   increment size per timestep of loop
 * @param L   lower bound
 * @param H   upper bound
 */
#define USLFL(X, C, A, L, H) {\
  C = X + A * float(random(3) - 1);\
  if (C < L)\
    C = L;\
  else if (C > H)\
    C = H;\
  X = C;\
}

/********** CAN VARIABLES **********/
/* Bus declaration */
can_bus_dev_t bus{};

/* Timer group for messages */
// constexpr uint8_t potentiometer_pin{34};
VirtualTimerGroup timer_group;

/* Sensor Variables (info) */
// HI to LO
// Increasing address locations
// Signals before messages
// Increasing start bit

/* HI Bus Level Sensors */
#ifdef CAN_BUS_HI
  // High Voltage BMS (values and signalling)
  CANSignal<float, 0, 12, CANTemplateConvertFloat(0.1), CANTemplateConvertFloat(0)> hv_max_discharge_current_sig;
  CANSignal<float, 12, 12, CANTemplateConvertFloat(0.1), CANTemplateConvertFloat(0)> hv_max_regen_current_sig;
  CANSignal<float, 24, 8, CANTemplateConvertFloat(0.01), CANTemplateConvertFloat(0)> hv_battery_voltage_sig;
  CANSignal<int8_t, 40, 8, CANTemplateConvertFloat(1), CANTemplateConvertFloat(-40)> hv_battery_temperature_sig;
  CANSignal<float, 48, 16, CANTemplateConvertFloat(0.01), CANTemplateConvertFloat(-40)> hv_battery_current_sig;

  CANTXMessage<5> hv_bms_soe_msg{bus, 0x240, 8, T_CS, timer_group,
    hv_max_discharge_current_sig,
    hv_max_regen_current_sig,
    hv_battery_voltage_sig,
    hv_battery_temperature_sig,
    hv_battery_current_sig
  };

  CANSignal<uint8_t, 40, 8, CANTemplateConvertFloat(1), CANTemplateConvertFloat(0)> hv_state_of_charge_sig;

  CANTXMessage<1> hv_bms_status_msg{bus, 0x241, 8, T_CS, timer_group, 
    hv_state_of_charge_sig
  };

  // Throttle
  CANSignal<uint8_t, 0, 8, CANTemplateConvertFloat(1), CANTemplateConvertFloat(0)> accel_percentage_sig;
  CANSignal<uint8_t, 8, 8, CANTemplateConvertFloat(1), CANTemplateConvertFloat(0)> brake_percentage_sig;

  CANTXMessage<2> throttle_values_msg{bus, 0x300, 2, T_CS, timer_group, 
    accel_percentage_sig,
    brake_percentage_sig
  };

  CANSignal<uint8_t /* enum */, 0, 8, CANTemplateConvertFloat(1), CANTemplateConvertFloat(0)> tractile_system_status_sig;

  CANTXMessage<1> throttle_status_msg{bus, 0x301, 8, T_CS, timer_group, 
    tractile_system_status_sig
  };

  // Temperature board
  CANSignal<float, 16, 16, CANTemplateConvertFloat(0.1), CANTemplateConvertFloat(-40)> coolant_temperature_sig;
  CANSignal<float, 32, 16, CANTemplateConvertFloat(0.1), CANTemplateConvertFloat(-40)> ambient_temperature_sig;

  CANTXMessage<2> ptrain_temp_msg{bus, 0x420, 6, T_CS, timer_group, 
    coolant_temperature_sig, 
    ambient_temperature_sig
  };

  CANSignal<float, 0, 16, CANTemplateConvertFloat(0.01), CANTemplateConvertFloat(0)> coolant_flow_sig;

  CANTXMessage<1> coolant_msg{bus, 0x421, 2, T_CS, timer_group, 
    coolant_flow_sig
  };
#endif

/* LO Bus Level Sensors */
#ifdef CAN_BUS_LO
  // Wheels (speed and brake temps)
  CANSignal<float, 0, 16, CANTemplateConvertFloat(0.1), CANTemplateConvertFloat(0)> fl_wheel_speed_sig;
  CANSignal<float, 16, 16, CANTemplateConvertFloat(0.1), CANTemplateConvertFloat(-40)> fl_brake_temperature_sig;

  CANTXMessage<2> fl_wheel_msg{bus, 0x400, 4, T_CS, timer_group, 
    fl_wheel_speed_sig, 
    fl_brake_temperature_sig
  };

  CANSignal<float, 0, 16, CANTemplateConvertFloat(0.1), CANTemplateConvertFloat(0)> fr_wheel_speed_sig;
  CANSignal<float, 16, 16, CANTemplateConvertFloat(0.1), CANTemplateConvertFloat(-40)> fr_brake_temperature_sig;

  CANTXMessage<2> fr_wheel_msg{bus, 0x401, 4, T_CS, timer_group, 
    fr_wheel_speed_sig, 
    fr_brake_temperature_sig
  };

  CANSignal<float, 0, 16, CANTemplateConvertFloat(0.1), CANTemplateConvertFloat(0)> bl_wheel_speed_sig;
  CANSignal<float, 16, 16, CANTemplateConvertFloat(0.1), CANTemplateConvertFloat(-40)> bl_brake_temperature_sig;

  CANTXMessage<2> bl_wheel_msg{bus, 0x402, 4, T_CS, timer_group, 
    bl_wheel_speed_sig, 
    bl_brake_temperature_sig
  };

  CANSignal<float, 0, 16, CANTemplateConvertFloat(0.1), CANTemplateConvertFloat(0)> br_wheel_speed_sig;
  CANSignal<float, 16, 16, CANTemplateConvertFloat(0.1), CANTemplateConvertFloat(-40)> br_brake_temperature_sig;

  CANTXMessage<2> br_wheel_msg{bus, 0x403, 4, T_CS, timer_group, 
    br_wheel_speed_sig, 
    br_brake_temperature_sig
  };

  // Brake Pressures
  CANSignal<uint16_t, 0, 16, CANTemplateConvertFloat(1), CANTemplateConvertFloat(0)> front_brake_pressure_sig;
  CANSignal<uint16_t, 16, 16, CANTemplateConvertFloat(1), CANTemplateConvertFloat(0)> rear_brake_pressure_sig;

  CANTXMessage<2> brake_pressure_msg{bus, 0x410, 4, T_CS, timer_group, 
    front_brake_pressure_sig, 
    rear_brake_pressure_sig
  };

  // Motion Board
  CANSignal<uint32_t, 0, 32, CANTemplateConvertFloat(1), CANTemplateConvertFloat(0)> latitude_sig;
  CANSignal<uint32_t, 32, 32, CANTemplateConvertFloat(1), CANTemplateConvertFloat(0)> longitude_sig;

  CANTXMessage<2> motion_gps_msg{bus, 0x430, 8, T_CS, timer_group, 
    latitude_sig, 
    longitude_sig
  };

  CANSignal<float, 0, 16, CANTemplateConvertFloat(0.0005), CANTemplateConvertFloat(0)> accel_x_sig;
  CANSignal<float, 16, 16, CANTemplateConvertFloat(0.0005), CANTemplateConvertFloat(0)> accel_y_sig;
  CANSignal<float, 32, 16, CANTemplateConvertFloat(0.0005), CANTemplateConvertFloat(0)> accel_z_sig;

  CANTXMessage<3> motion_accelerometer_msg{bus, 0x431, 6, T_CS, timer_group, 
    accel_x_sig, 
    accel_y_sig, 
    accel_z_sig
  };

  CANSignal<float, 0, 16, CANTemplateConvertFloat(0.0005), CANTemplateConvertFloat(0)> gyro_x_sig;
  CANSignal<float, 16, 16, CANTemplateConvertFloat(0.0005), CANTemplateConvertFloat(0)> gyro_y_sig;
  CANSignal<float, 32, 16, CANTemplateConvertFloat(0.0005), CANTemplateConvertFloat(0)> gyro_z_sig;

  CANTXMessage<3> motion_gyroscope_msg{bus, 0x432, 6, T_CS, timer_group, 
    gyro_x_sig, 
    gyro_y_sig, 
    gyro_z_sig
  };

  // RTC
  CANSignal<uint32_t, 0, 32, CANTemplateConvertFloat(1), CANTemplateConvertFloat(0)> rtc_sig;

  CANTXMessage<1> rtc_msg{bus, 0x430, 8, T_CS, timer_group, 
    rtc_sig
  };
#endif

/********** SIM VARS **********/
// Temporary values with primitive types to allow for operation type matching
float temp_fl;

/********** FUNCTIONS **********/
// Each function is segmented according to which HI or LO sensors are available

/**
 * @brief Initilizes all sensor values
 * This keeps all data within specified bounds of sim and sets up all const values at once
 * Sim specs can be found here: https://docs.google.com/spreadsheets/d/1ynXjzQ_-UCvqHWCWHrc_tA7Y1Py2moUgZKMmyAJMJCY/edit#gid=0
 */
void sim_init() {
#ifdef CAN_BUS_HI
  hv_battery_voltage_sig = 0;
  hv_battery_temperature_sig = 0;
  coolant_temperature_sig = 90.0f;
  hv_state_of_charge_sig = 4;
  tractile_system_status_sig = 0;
  ambient_temperature_sig = 77.0f;
  accel_percentage_sig = 0;
  brake_percentage_sig = 0;
  hv_battery_current_sig = 0.0f;
  hv_max_discharge_current_sig = 0.0f;
  hv_max_regen_current_sig = 0.0f;
  coolant_flow_sig = 55.0f;
#endif

#ifdef CAN_BUS_LO
  fl_wheel_speed_sig = 3.4f;
  fr_wheel_speed_sig = 3.4f;
  bl_wheel_speed_sig = 3.4f;
  br_wheel_speed_sig = 3.4f;
  front_brake_pressure_sig = 0;
  rear_brake_pressure_sig = 0;
  fl_brake_temperature_sig = 85.0f;
  fr_brake_temperature_sig = 85.0f;
  bl_brake_temperature_sig = 85.0f;
  br_brake_temperature_sig = 85.0f;
  accel_x_sig = 13.0f;
  accel_y_sig = -3.0f;
  accel_z_sig = 7.0f;
  gyro_x_sig = 0.0f;
  gyro_y_sig = 0.0f;
  gyro_z_sig = 0.0f;
  latitude_sig = 69;
  longitude_sig = 69;
  rtc_sig = 1337;
#endif
}

/**
 * @brief Perform fast-timed updates
 * 
 */
void update_fast() {
#ifdef CAN_BUS_HI
  USLINT(hv_battery_temperature_sig, -10, 10);
  C_SAW(hv_battery_current_sig, temp_fl, 1.0f, -99.0f, 99.0f);
#endif

#ifdef CAN_BUS_LO
  USLFL(fl_wheel_speed_sig, temp_fl, 0.7f, 2.0f, 9.0f);
  USLFL(fr_wheel_speed_sig, temp_fl, 0.7f, 2.0f, 9.0f);
  USLFL(bl_wheel_speed_sig, temp_fl, 0.7f, 2.0f, 9.0f);
  USLFL(br_wheel_speed_sig, temp_fl, 0.7f, 2.0f, 9.0f);
  Q_SAW(front_brake_pressure_sig, 0, 63);
#endif

  // bus.Tick();
}

/**
 * @brief Perform slow-timed updates
 * 
 */
void update_slow() {
#ifdef CAN_BUS_HI
  USLFL(coolant_temperature_sig, temp_fl, 0.2f, 89.0f, 91.0f);
  USLINT(hv_state_of_charge_sig, 2, 9);
  C_SAW(ambient_temperature_sig, temp_fl, -1.0f, 76.0f, 80.0f);
  USLFL(coolant_flow_sig, temp_fl, 0.7f, 50.0f, 60.0f);
#endif

#ifdef CAN_BUS_LO
  C_SAW(fl_brake_temperature_sig, temp_fl, 0.3f, 85.0f, 95.0f);
  C_SAW(fr_brake_temperature_sig, temp_fl, 0.3f, 85.0f, 95.0f);
  C_SAW(bl_brake_temperature_sig, temp_fl, 0.3f, 85.0f, 95.0f);
  C_SAW(br_brake_temperature_sig, temp_fl, 0.3f, 85.0f, 95.0f);
#endif

  // bus.Tick();
}

/********** PROGRAM **********/
void setup()
{
  /* Initialize Serial and CAN bus */
  Serial.begin(9600);
  // Serial.println("Started");
  bus.Initialize(ICAN::BaudRate::kBaud1M);

  /* Set starting signal values */
  sim_init();

  /* Add update timers */
  timer_group.AddTimer(T_CS, update_fast);
  timer_group.AddTimer(T_SEC, update_slow);
}

void loop()
{
  /* Update */
  timer_group.Tick(millis());
  
  /* Test: print serial data */
  // 8 at a time
  // #ifdef CAN_BUS_HI
  //   Serial.print(hv_battery_voltage_sig); Serial.print(",\t");
  //   Serial.print(hv_battery_temperature_sig); Serial.print(",\t");
  //   Serial.print(tractile_system_status_sig); Serial.print(",\t");
  //   Serial.print(accel_percentage_sig); Serial.print(",\t");
  //   Serial.print(brake_percentage_sig); Serial.print(",\t");
  //   Serial.print(brake_percentage_sig); Serial.print(",\t");
  //   Serial.print(hv_battery_current_sig); Serial.print(",\t");
  //   Serial.print(hv_max_discharge_current_sig); Serial.print(",\t");
  //   Serial.println(hv_max_regen_current_sig);

  //   Serial.print(coolant_temperature_sig); Serial.print(",\t");
  //   Serial.print(hv_state_of_charge_sig); Serial.print(",\t");
  //   Serial.print(ambient_temperature_sig); Serial.print(",\t");
  //   Serial.println(coolant_flow_sig);
  // #endif

  // #ifdef CAN_BUS_LO
  //   Serial.print(fl_wheel_speed_sig); Serial.print(",\t");
  //   Serial.print(fr_wheel_speed_sig); Serial.print(",\t");
  //   Serial.print(bl_wheel_speed_sig); Serial.print(",\t");
  //   Serial.print(br_wheel_speed_sig); Serial.print(",\t");
  //   Serial.print(front_brake_pressure_sig); Serial.print(",\t");
  //   Serial.println(rear_brake_pressure_sig);

  //   Serial.print(fl_brake_temperature_sig); Serial.print(",\t");
  //   Serial.print(fr_brake_temperature_sig); Serial.print(",\t");
  //   Serial.print(bl_brake_temperature_sig); Serial.print(",\t");
  //   Serial.print(br_brake_temperature_sig); Serial.print(",\t");
  //   Serial.println(rtc_sig);
  // #endif

  /* Send through CAN */
}