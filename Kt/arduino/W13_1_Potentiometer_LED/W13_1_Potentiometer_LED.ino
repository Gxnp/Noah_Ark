// งาน 13.1 : Potentiometer Analog Reading -> ปรับความสว่าง LED
// บอร์ด Arduino UNO R4 WiFi
// วงจร: Potentiometer ขาริม -> 5V และ GND, ขากลาง -> A0
//       LED: D3 (PWM ~) -> R 220Ω -> ขายาว, ขาสั้น -> GND

const int POT_PIN = A0;
const int LED_PIN = 3;     // ต้องเป็นขา PWM (~): 3, 5, 6, 9, 10, 11

void setup() {
  Serial.begin(9600);
  pinMode(LED_PIN, OUTPUT);
}

void loop() {
  int raw = analogRead(POT_PIN);                 // 0–1023 (ค่าเริ่มต้น 10 บิต)
  int brightness = map(raw, 0, 1023, 0, 255);    // แปลงเป็น PWM 0–255
  analogWrite(LED_PIN, brightness);

  Serial.print("Pot = ");
  Serial.print(raw);
  Serial.print("  ->  LED = ");
  Serial.println(brightness);
  delay(50);
}
