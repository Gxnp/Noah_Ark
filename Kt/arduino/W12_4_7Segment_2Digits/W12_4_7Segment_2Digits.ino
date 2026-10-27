// งาน 12.4 : 7-Segment 2 หลัก แสดงรหัสนักศึกษาทีละ 2 หลัก ช่วงละ 1 วินาที
// เช่น รหัส 68070180 -> แสดง 66, 07, 00, 12
// บอร์ด Arduino UNO R4 WiFi   เทคนิค: Multiplexing
// วงจร (Common Cathode 2 ตัว):
//   ขา a–g ของทั้ง 2 ตัวต่อขนานกัน -> D2–D8 (ผ่าน R 220Ω)
//   Common ตัวซ้าย (หลักสิบ) -> D11, Common ตัวขวา (หลักหน่วย) -> D12

const char STUDENT_ID[] = "68070180";             // <<< ใส่รหัสนักศึกษาของตัวเอง

const int SEG_PINS[7]   = {2, 3, 4, 5, 6, 7, 8};  // a, b, c, d, e, f, g
const int DIGIT_PINS[2] = {11, 12};               // [0] = ตัวซ้าย (หลักสิบ), [1] = ตัวขวา (หลักหน่วย)
const bool COMMON_ANODE = false;

const byte DIGITS[10][7] = {
  {1,1,1,1,1,1,0}, {0,1,1,0,0,0,0}, {1,1,0,1,1,0,1}, {1,1,1,1,0,0,1}, {0,1,1,0,0,1,1},
  {1,0,1,1,0,1,1}, {1,0,1,1,1,1,1}, {1,1,1,0,0,0,0}, {1,1,1,1,1,1,1}, {1,1,1,1,0,1,1}
};

void writeSegments(int n) {
  for (int i = 0; i < 7; i++) {
    bool on = DIGITS[n][i];
    if (COMMON_ANODE) on = !on;
    digitalWrite(SEG_PINS[i], on ? HIGH : LOW);
  }
}

// Common Cathode: Common = LOW คือเปิดตัวนั้น, HIGH คือปิด
void digitOn(int d)  { digitalWrite(DIGIT_PINS[d], COMMON_ANODE ? HIGH : LOW); }
void digitOff(int d) { digitalWrite(DIGIT_PINS[d], COMMON_ANODE ? LOW : HIGH); }

// แสดงเลข 2 หลักค้างไว้ ms มิลลิวินาที โดยสลับเปิดทีละตัวเร็ว ๆ
void showTwoDigits(int tens, int ones, unsigned long ms) {
  unsigned long start = millis();
  while (millis() - start < ms) {
    digitOff(1); writeSegments(tens); digitOn(0); delay(5);   // ตัวซ้าย
    digitOff(0); writeSegments(ones); digitOn(1); delay(5);   // ตัวขวา
  }
  digitOff(0);
  digitOff(1);
}

void setup() {
  for (int i = 0; i < 7; i++) pinMode(SEG_PINS[i], OUTPUT);
  for (int d = 0; d < 2; d++) { pinMode(DIGIT_PINS[d], OUTPUT); digitOff(d); }
}

void loop() {
  int len = strlen(STUDENT_ID);
  for (int i = 0; i + 1 < len; i += 2) {
    int tens = STUDENT_ID[i] - '0';        // แปลงตัวอักษร '6' เป็นตัวเลข 6
    int ones = STUDENT_ID[i + 1] - '0';
    showTwoDigits(tens, ones, 1000);
    delay(150);                            // ดับสั้น ๆ ให้เห็นว่าเปลี่ยนคู่ (เช่น 00 ต่อ 00)
  }
}
