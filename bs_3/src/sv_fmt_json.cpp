#include "sv_fmt_json.h"

void sv_fmt_json(message_code_t* mc, sensor_vals_t* sv) {
  Serial.print("{\"fast\":{\"fl_wheel_speed\":"); Serial.print(sv->fast.fl_wheel_speed);
  Serial.print(",\"fr_wheel_speed\":"); Serial.print(sv->fast.fr_wheel_speed);
  Serial.print(",\"bl_wheel_speed\":"); Serial.print(sv->fast.bl_wheel_speed);
  Serial.print(",\"br_wheel_speed\":"); Serial.print(sv->fast.br_wheel_speed);
  Serial.print(",\"hv_battery_voltage\":"); Serial.print(sv->fast.hv_battery_voltage);
  Serial.print(",\"motor_temperature\":"); Serial.print(sv->fast.motor_temperature);
  Serial.print(",\"accel_x\":"); Serial.print(sv->fast.accel_x);
  Serial.print(",\"accel_y\":"); Serial.print(sv->fast.accel_y);
  Serial.print(",\"accel_z\":"); Serial.print(sv->fast.accel_z);
  Serial.print(",\"gyro_x\":"); Serial.print(sv->fast.gyro_x);
  Serial.print(",\"gyro_y\":"); Serial.print(sv->fast.gyro_y);
  Serial.print(",\"gyro_z\":"); Serial.print(sv->fast.gyro_z);
  Serial.print(",\"latitude\":"); Serial.print(sv->fast.latitude);
  Serial.print(",\"longitude\":"); Serial.print(sv->fast.longitude);
  Serial.print(",\"rpm\":"); Serial.print(sv->fast.rpm);
  Serial.print(",\"hv_battery_current\":"); Serial.print(sv->fast.hv_battery_current);
  Serial.print(",\"hv_max_discharge_current\":"); Serial.print(sv->fast.hv_max_discharge_current);
  Serial.print(",\"hv_max_regen_current\":"); Serial.print(sv->fast.hv_max_regen_current);
  Serial.print(",\"rtc\":"); Serial.print(sv->fast.rtc);
  Serial.print(",\"front_brake_pressure\":"); Serial.print(sv->fast.front_brake_pressure);
  Serial.print(",\"rear_brake_pressure\":"); Serial.print(sv->fast.rear_brake_pressure);
  Serial.print(",\"hv_battery_temperature\":"); Serial.print(sv->fast.hv_battery_temperature);
  Serial.print(",\"tractile_system_status\":"); Serial.print(sv->fast.tractile_system_status);
  Serial.print(",\"accel_percentage\":"); Serial.print(sv->fast.accel_percentage);
  Serial.print(",\"brake_percentage\":"); Serial.print(sv->fast.brake_percentage);
  Serial.print("},\"slow\":{\"coolant_temperature\":"); Serial.print(sv->slow.coolant_temperature);
  Serial.print(",\"fl_brake_temperature\":"); Serial.print(sv->slow.fl_brake_temperature);
  Serial.print(",\"fr_brake_temperature\":"); Serial.print(sv->slow.fr_brake_temperature);
  Serial.print(",\"bl_brake_temperature\":"); Serial.print(sv->slow.bl_brake_temperature);
  Serial.print(",\"br_brake_temperature\":"); Serial.print(sv->slow.br_brake_temperature);
  Serial.print(",\"ambient_temperature\":"); Serial.print(sv->slow.ambient_temperature);
  Serial.print(",\"inverter_temperature\":"); Serial.print(sv->slow.inverter_temperature);
  Serial.print(",\"coolant_flow\":"); Serial.print(sv->slow.coolant_flow);
  Serial.print(",\"hv_state_of_charge\":"); Serial.print(sv->slow.hv_state_of_charge);
  Serial.println("}");
}