/********** INCLUDES **********/
#include <Arduino.h>
#include "sv_fmt_json.h"

/********** VARIABLES **********/
// Test data
sensor_vals_t test_sensors;
message_code_t mc = 0;

// Counter for loop
uint16_t counter = 0;

/********** PROGRAM **********/
void setup() {
  // Initialize data
  test_sensors = {
      .fast = {
        .fl_wheel_speed = 31.0, // All `wheel_speed`s vary based on CAN_Mock_Test's algorithm
        .fl_brake_temperature = 420.0, // All `brake_temperature`s are constant
        .fr_wheel_speed = 41.0,
        .fr_brake_temperature = 420.0,
        .bl_wheel_speed = 59.0,
        .bl_brake_temperature = 420.0,
        .br_wheel_speed = 26.0,
        .br_brake_temperature = 420.0,
        .front_brake_pressure = 0, // Constant 0
        .rear_brake_pressure = 0, // Random increment
        .packetnum = counter // Constant increment
      },
      .slow = {
        .fake_value = 69 // Slower increment
      },
      .control = 0 // Constant, may manually alter later
    };
}

void loop() {
  // Print data in JSON format to Serial
  sv_fmt_json(&mc, &test_sensors);

  // Increment counter and control loopback
  ++counter;
  delay(100);

  // Simulate updates: modify data
  test_sensors.fast.fl_wheel_speed = random(max(1.0, test_sensors.fast.fl_wheel_speed - 1.0) * 1000,
                                            min(90.0, test_sensors.fast.fl_wheel_speed + 1.0) * 1000) / 1000.0f;
  test_sensors.fast.fr_wheel_speed = random(max(1.0, test_sensors.fast.fr_wheel_speed - 1.0) * 1000, 
                                            min(90.0, test_sensors.fast.fr_wheel_speed + 1.0) * 1000) / 1000.0f;
  test_sensors.fast.bl_wheel_speed = random(max(1.0, test_sensors.fast.bl_wheel_speed - 1.0) * 1000, 
                                            min(90.0, test_sensors.fast.bl_wheel_speed + 1.0) * 1000) / 1000.0f;
  test_sensors.fast.br_wheel_speed = random(max(1.0, test_sensors.fast.br_wheel_speed - 1.0) * 1000, 
                                            min(90.0, test_sensors.fast.br_wheel_speed + 1.0) * 1000) / 1000.0f;
  test_sensors.fast.rear_brake_pressure += random(0, 2);
  test_sensors.fast.packetnum = counter;
  if (counter % 10 == 0) {
    ++test_sensors.slow.fake_value;
  }
}
