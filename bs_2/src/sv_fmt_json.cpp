#include "sv_fmt_json.h"

void sv_fmt_json(message_code_t* mc, sensor_vals_t* sv) {
  Serial.print("{\"fast\":{\"fl_wheel_speed\":"); Serial.print(sv->fast.fl_wheel_speed);
  Serial.print(",\"fl_brake_temperature\":"); Serial.print(sv->fast.fl_brake_temperature);
  Serial.print(",\"fr_wheel_speed\":"); Serial.print(sv->fast.fr_wheel_speed);
  Serial.print(",\"fr_brake_temperature\":"); Serial.print(sv->fast.fr_brake_temperature);
  Serial.print(",\"bl_wheel_speed\":"); Serial.print(sv->fast.bl_wheel_speed);
  Serial.print(",\"bl_brake_temperature\":"); Serial.print(sv->fast.bl_brake_temperature);
  Serial.print(",\"br_wheel_speed\":"); Serial.print(sv->fast.br_wheel_speed);
  Serial.print(",\"br_brake_temperature\":"); Serial.print(sv->fast.br_brake_temperature);
  Serial.print(",\"front_brake_pressure\":"); Serial.print(sv->fast.front_brake_pressure);
  Serial.print(",\"rear_brake_pressure\":"); Serial.print(sv->fast.rear_brake_pressure);
  Serial.print(",\"packetnum\":"); Serial.print(sv->fast.packetnum);
  Serial.print("},\"slow\":{\"fake_value\":"); Serial.print(sv->slow.fake_value);
  Serial.print("},\"control\":"); Serial.print(sv->control);
  Serial.println("}");
}