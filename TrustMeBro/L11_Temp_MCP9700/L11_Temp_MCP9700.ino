void setup() {
  Serial.begin(9600);
}

void loop() {
  int rawADC = analogRead(A0);

  // แปลงค่า ADC (0-1023) เป็นแรงดัน (V)
  float voltage = (rawADC * 5.0) / 1023.0;

  // คำนวณอุณหภูมิสำหรับ MCP9700 (Vout - 0.5V) / 0.01V
  float tempC = (voltage - 0.5) * 100.0;

  Serial.print("Voltage: ");
  Serial.print(voltage, 3);
  Serial.print(" V | Temp: ");
  Serial.print(tempC, 1);
  Serial.println(" °C");

  delay(500);
}
