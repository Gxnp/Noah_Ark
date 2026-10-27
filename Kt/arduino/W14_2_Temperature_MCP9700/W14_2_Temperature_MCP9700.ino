// งาน 14.2 : Temperature Sensor MCP9700 แสดงอุณหภูมิทาง Serial Monitor
// บอร์ด Arduino UNO R4 WiFi
// วงจร (หันด้านแบนที่มีตัวหนังสือเข้าหาตัว): ขาซ้าย VDD -> 5V, ขากลาง Vout -> A0, ขาขวา GND -> GND
// ใน Tinkercad ใช้ชิ้น "Temperature Sensor [TMP36]" แทนได้ สูตรเดียวกัน

const int TEMP_PIN = A0;

void setup() {
  Serial.begin(9600);
}

void loop() {
  int raw = analogRead(TEMP_PIN);            // 0–1023
  float voltage = raw * 5.0 / 1023.0;        // แปลงเป็นโวลต์

  // MCP9700: 0 °C = 0.5 V และเพิ่ม 10 mV ต่อ 1 °C
  float tempC = (voltage - 0.5) / 0.01;

  Serial.print("ADC = ");
  Serial.print(raw);
  Serial.print("  V = ");
  Serial.print(voltage, 3);
  Serial.print(" V  Temp = ");
  Serial.print(tempC, 1);
  Serial.println(" C");
  delay(1000);
}
