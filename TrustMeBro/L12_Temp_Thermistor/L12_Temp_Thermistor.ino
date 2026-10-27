#include <math.h>

const int   THERMISTOR_PIN = A0;
const float T0       = 298.15;   // 25°C (Kelvin)
const float R0       = 10000.0;  // 10k ที่ 25°C
const float B_VALUE  = 4050.0;   // B25/50
const float R_FIXED  = 10000.0;  // ตัวต้านทานอนุกรม 10k
const float ADC_MAX  = 1023.0;

float readCelsius() {
  long sum = 0;
  for (int i = 0; i < 20; i++) {
    sum += analogRead(THERMISTOR_PIN);
    delay(5);
  }
  float adc = sum / 20.0;

  // Thermistor อยู่ระหว่าง A0 กับ GND
  float r = R_FIXED * adc / (ADC_MAX - adc);

  float kelvin = 1.0 / ((1.0 / T0) + (log(r / R0) / B_VALUE));
  return kelvin - 273.15;
}

void setup() {
  Serial.begin(9600);
}

void loop() {
  Serial.println(readCelsius(), 2);
  delay(500);
}
