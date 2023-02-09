#include "sv_fmt_json.h"

void sv_fmt_json(message_code_t* mc, sensor_vals_t* sv) {
  Serial.print("{\"fast\":{\"jigglypuff\":"); Serial.print(sv->fast.jigglypuff);
  Serial.print(",\"sheik\":"); Serial.print(sv->fast.sheik);
  Serial.print(",\"greninja\":"); Serial.print(sv->fast.greninja);
  Serial.print("},{\"med\":{\"sunset\":"); Serial.print(sv->med.sunset);
  Serial.print(",\"twilight\":"); Serial.print(sv->med.twilight);
  Serial.print("},{\"slow\":{\"fake_value\":"); Serial.print(sv->slow.fake_value);
  Serial.print("},\"exodia\":"); Serial.print(sv->exodia);
  Serial.print(",\"ragnar\":"); Serial.print(sv->ragnar);
  Serial.println("}");
}