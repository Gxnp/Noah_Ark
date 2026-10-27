// =====================================================================
// สไลด์ PC69-13 แบบฝึกหัดที่ 1 : Keypad + 7-Segment
// ต่อ Keypad แบบ internal pullup แล้วแสดงตัวเลขบน 7-Segment ตามปุ่มที่กด
//   กด 0–9 -> แสดงเลขนั้น,  กด * -> ดับ,  กด A B C D # -> แสดงขีดกลาง
//
// บอร์ด Arduino UNO R4 WiFi   ไลบรารี: Adafruit Keypad
// วงจร Keypad (ตามสไลด์): D2–D5 = Column (C4..C1), D6–D9 = Row (R4..R1)
//      R1 -> D9, R2 -> D8, R3 -> D7, R4 -> D6, C1 -> D5, C2 -> D4, C3 -> D3, C4 -> D2
// วงจร 7-Segment (Common Cathode) ย้ายไปขาที่เหลือ:
//      a -> D10, b -> D11, c -> D12, d -> D13, e -> A0, f -> A1, g -> A2 (R 220–330Ω), Common -> GND
// =====================================================================

#include "Adafruit_Keypad.h"

const byte ROWS = 4, COLS = 4;
char keys[ROWS][COLS] = {
  {'1', '2', '3', 'A'},
  {'4', '5', '6', 'B'},
  {'7', '8', '9', 'C'},
  {'*', '0', '#', 'D'}
};
byte rowPins[ROWS] = {9, 8, 7, 6};
byte colPins[COLS] = {5, 4, 3, 2};
Adafruit_Keypad keypad = Adafruit_Keypad(makeKeymap(keys), rowPins, colPins, ROWS, COLS);

const int SEG_PINS[7] = {10, 11, 12, 13, A0, A1, A2};   // a b c d e f g
const bool COMMON_ANODE = false;
const byte DIGITS[10][7] = {
  {1,1,1,1,1,1,0}, {0,1,1,0,0,0,0}, {1,1,0,1,1,0,1}, {1,1,1,1,0,0,1}, {0,1,1,0,0,1,1},
  {1,0,1,1,0,1,1}, {1,0,1,1,1,1,1}, {1,1,1,0,0,0,0}, {1,1,1,1,1,1,1}, {1,1,1,1,0,1,1}
};

void writeSeg(int i, bool on) { if (COMMON_ANODE) on = !on; digitalWrite(SEG_PINS[i], on ? HIGH : LOW); }
void showDigit(int n) { for (int i = 0; i < 7; i++) writeSeg(i, DIGITS[n][i]); }
void showDash()       { for (int i = 0; i < 7; i++) writeSeg(i, i == 6); }
void blank()          { for (int i = 0; i < 7; i++) writeSeg(i, false); }

void setup() {
  Serial.begin(9600);
  keypad.begin();                                     // ตั้งขา keypad (internal pullup ในไลบรารี)
  for (int i = 0; i < 7; i++) pinMode(SEG_PINS[i], OUTPUT);
  blank();
}

void loop() {
  keypad.tick();
  while (keypad.available()) {
    keypadEvent e = keypad.read();
    if (e.bit.EVENT == KEY_JUST_PRESSED) {
      char k = (char)e.bit.KEY;
      Serial.print("Pressed: ");
      Serial.println(k);
      if (k >= '0' && k <= '9') showDigit(k - '0');
      else if (k == '*')        blank();
      else                      showDash();
    }
  }
  delay(10);
}
