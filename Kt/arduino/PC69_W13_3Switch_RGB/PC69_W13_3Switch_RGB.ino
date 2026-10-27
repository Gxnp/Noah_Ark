// =====================================================================
// สไลด์ PC69-13 การทดลองที่ 3 : Digital Input — Switch 3 ตัว
// กด switch แต่ละตัว ให้ Color LED ติดคนละสี (Red, Green, Blue) ปล่อยแล้วดับ
//
// บอร์ด Arduino UNO R4 WiFi (ไม่ต้องใช้ WiFi)
// วงจร Switch: ขาหนึ่ง -> D2 / D3 / D4, ขาทแยง -> GND  (ใช้ INPUT_PULLUP: กด = LOW)
//      Color LED (Common Anode): Common -> 5V, R -> D9, B -> D10, G -> D11 (R 220–330Ω)
// =====================================================================

const int SW_PIN[3]  = {2, 3, 4};      // Switch 1, 2, 3
const int R_PIN = 9, B_PIN = 10, G_PIN = 11;
const bool COMMON_ANODE = true;        // Color LED ที่แจกเป็น Common Anode

void setRgb(bool r, bool g, bool b) {
  if (COMMON_ANODE) { r = !r; g = !g; b = !b; }
  digitalWrite(R_PIN, r);
  digitalWrite(G_PIN, g);
  digitalWrite(B_PIN, b);
}

void setup() {
  Serial.begin(9600);
  for (int i = 0; i < 3; i++) pinMode(SW_PIN[i], INPUT_PULLUP);   // internal pull up
  pinMode(R_PIN, OUTPUT);
  pinMode(G_PIN, OUTPUT);
  pinMode(B_PIN, OUTPUT);
  setRgb(0, 0, 0);
}

void loop() {
  bool s1 = digitalRead(SW_PIN[0]) == LOW;   // กด = LOW
  bool s2 = digitalRead(SW_PIN[1]) == LOW;
  bool s3 = digitalRead(SW_PIN[2]) == LOW;

  // กดพร้อมกันหลายตัวได้ สีจะผสมกัน เช่น 1+3 = ม่วง
  setRgb(s1, s2, s3);

  static int last = -1;
  int state = s1 * 1 + s2 * 2 + s3 * 4;
  if (state != last) {
    Serial.print("SW1=");
    Serial.print(s1);
    Serial.print(" SW2=");
    Serial.print(s2);
    Serial.print(" SW3=");
    Serial.println(s3);
    last = state;
  }
  delay(20);
}
