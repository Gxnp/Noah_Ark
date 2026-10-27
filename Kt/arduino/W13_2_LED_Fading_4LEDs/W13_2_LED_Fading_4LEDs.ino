// งาน 13.2 : Analog LED Fading 4 ดวง
// บอร์ด Arduino UNO R4 WiFi
//   LED1 : Off -> Full-Bright
//   LED2 : Off -> Full-Bright เร็วเป็น 2 เท่าของ LED1
//   LED3 : Full-Bright -> Off
//   LED4 : Half-Bright -> Full-Bright
// วงจร: LED1 -> D3, LED2 -> D5, LED3 -> D6, LED4 -> D9 (ทุกดวงผ่าน R 220Ω, ขาสั้น -> GND)

const int LED1 = 3;
const int LED2 = 5;
const int LED3 = 6;
const int LED4 = 9;

const int STEP_DELAY = 10;   // ms ต่อ 1 ขั้น -> LED1 ใช้ 256 ขั้น ≈ 2.56 วินาทีต่อรอบ

void setup() {
  pinMode(LED1, OUTPUT);
  pinMode(LED2, OUTPUT);
  pinMode(LED3, OUTPUT);
  pinMode(LED4, OUTPUT);
}

void loop() {
  for (int step = 0; step <= 255; step++) {
    analogWrite(LED1, step);                 // 0 -> 255
    analogWrite(LED2, (step * 2) % 256);     // 0 -> 255 สองรอบ ในเวลาเท่ากับ LED1 หนึ่งรอบ
    analogWrite(LED3, 255 - step);           // 255 -> 0
    analogWrite(LED4, 128 + step / 2);       // 128 (ครึ่ง) -> 255 (เต็ม)
    delay(STEP_DELAY);
  }
}
