// งาน 14.3 : Thermistor วัดอุณหภูมิ แสดงผลเทียบกับ MCP9700 ทาง Serial Monitor
// บอร์ด Arduino UNO R4 WiFi
// วงจร Thermistor (ตามสไลด์): 5V -> R 10kΩ -> จุดกลาง (A1) -> Thermistor -> GND
//   Vout = 5V × Rth / (10k + Rth)
// วงจร MCP9700: VDD -> 5V, Vout -> A0, GND -> GND

#include <math.h>

const int   THERM_PIN = A1;
const int   MCP_PIN   = A0;

const float R_FIXED = 10000.0;   // ตัวต้านทานคงที่ 10kΩ (R1)
const float R0      = 10000.0;   // ความต้านทาน Thermistor ที่ 25 °C (NTC 10k)
const float BETA    = 4050.0;    // ค่า B ของ Thermistor (TTC05103 = 4050)
const float T0      = 298.15;    // 25 °C ในหน่วยเคลวิน

float readThermistorC() {
  int raw = analogRead(THERM_PIN);
  if (raw <= 0 || raw >= 1023) return NAN;          // สายหลุด / ต่อผิด

  // จาก Vout/Vin = Rth/(R+Rth) -> Rth = R × raw / (1023 − raw)
  float rTh = R_FIXED * raw / (1023.0 - raw);

  // สมการ Beta: 1/T = 1/T0 + ln(Rth/R0) / B   (T เป็นเคลวิน)
  float tempK = 1.0 / (1.0 / T0 + log(rTh / R0) / BETA);
  return tempK - 273.15;
}

float readMcp9700C() {
  float v = analogRead(MCP_PIN) * 5.0 / 1023.0;
  return (v - 0.5) / 0.01;
}

void setup() {
  Serial.begin(9600);
}

void loop() {
  float tTherm = readThermistorC();
  float tMcp   = readMcp9700C();

  Serial.print("Thermistor = ");
  if (isnan(tTherm)) Serial.print("check wiring");
  else { Serial.print(tTherm, 1); Serial.print(" C"); }

  Serial.print("   |   MCP9700 = ");
  Serial.print(tMcp, 1);
  Serial.print(" C   |   diff = ");
  Serial.print(tTherm - tMcp, 1);
  Serial.println(" C");
  delay(1000);
}
