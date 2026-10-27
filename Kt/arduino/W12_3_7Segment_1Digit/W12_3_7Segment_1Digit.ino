// งาน 12.3 : 7-Segment 1 หลัก แสดงเลข 0–9 ช่วงละ 1 วินาที
// บอร์ด Arduino UNO R4 WiFi
// วงจร (Common Cathode ตามโจทย์):
//   a (ขา 7) -> D2   b (ขา 6) -> D3   c (ขา 4) -> D4   d (ขา 2) -> D5
//   e (ขา 1) -> D6   f (ขา 9) -> D7   g (ขา 10) -> D8  (ผ่าน R 220Ω ทุกเส้น)
//   Common (ขา 3 หรือ 8) -> GND

const int SEG_PINS[7] = {2, 3, 4, 5, 6, 7, 8};   // a, b, c, d, e, f, g
const bool COMMON_ANODE = false;                  // ถ้าเป็น Common Anode ให้เป็น true และต่อ Common -> 5V

//          a
//        -----
//     f |     | b
//        --g--
//     e |     | c
//        -----
//          d
// แต่ละแถว = ส่วน a b c d e f g (1 = ติด)
const byte DIGITS[10][7] = {
  {1,1,1,1,1,1,0},  // 0
  {0,1,1,0,0,0,0},  // 1
  {1,1,0,1,1,0,1},  // 2
  {1,1,1,1,0,0,1},  // 3
  {0,1,1,0,0,1,1},  // 4
  {1,0,1,1,0,1,1},  // 5
  {1,0,1,1,1,1,1},  // 6
  {1,1,1,0,0,0,0},  // 7
  {1,1,1,1,1,1,1},  // 8
  {1,1,1,1,0,1,1}   // 9
};

void showDigit(int n) {
  for (int i = 0; i < 7; i++) {
    bool on = DIGITS[n][i];
    if (COMMON_ANODE) on = !on;
    digitalWrite(SEG_PINS[i], on ? HIGH : LOW);
  }
}

void setup() {
  for (int i = 0; i < 7; i++) pinMode(SEG_PINS[i], OUTPUT);
}

void loop() {
  for (int n = 0; n <= 9; n++) {
    showDigit(n);
    delay(1000);
  }
}
