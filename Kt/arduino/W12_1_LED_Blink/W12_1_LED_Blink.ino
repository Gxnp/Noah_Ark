// งาน 12.1 : ไฟกระพริบ (กำหนดความถี่เอง)
// บอร์ด Arduino UNO R4 WiFi
// วงจร: D13 -> R 220Ω -> ขายาว (+) ของ LED, ขาสั้น (−) -> GND

const int   LED_PIN  = 13;
const float BLINK_HZ = 2.0;    // ความถี่ = กระพริบกี่ครั้งต่อวินาที (เปลี่ยนได้ เช่น 0.5, 1, 5)

unsigned long halfPeriod;      // เวลาติด = เวลาดับ = ครึ่งคาบ (ms)

void setup() {
  Serial.begin(9600);
  pinMode(LED_PIN, OUTPUT);

  // คาบ T = 1 / f วินาที -> ติดครึ่งคาบ ดับครึ่งคาบ
  halfPeriod = (unsigned long)(1000.0 / BLINK_HZ / 2.0);
  Serial.print("Blink ");
  Serial.print(BLINK_HZ);
  Serial.print(" Hz, on/off = ");
  Serial.print(halfPeriod);
  Serial.println(" ms");
}

void loop() {
  digitalWrite(LED_PIN, HIGH);
  delay(halfPeriod);
  digitalWrite(LED_PIN, LOW);
  delay(halfPeriod);
}
