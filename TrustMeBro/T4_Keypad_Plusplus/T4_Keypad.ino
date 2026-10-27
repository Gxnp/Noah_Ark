#include <LiquidCrystal_I2C.h>
#include <Keypad.h>

LiquidCrystal_I2C lcd(0x27, 16, 2);

const byte ROWS = 4;
const byte COLS = 4;

char hexaKeys[ROWS][COLS] = {
  {'1', '2', '3', 'A'},
  {'4', '5', '6', 'B'},
  {'7', '8', '9', 'C'},
  {'*', '0', '#', 'D'}
};

byte rowPins[ROWS] = {12, 11, 10, 9};
byte colPins[COLS] = {8, 7, 6, 5};

Keypad customKeypad = Keypad(makeKeymap(hexaKeys), rowPins, colPins, ROWS, COLS);

void resetScreen() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Input: ");
  lcd.setCursor(0, 1); // นำเคอร์เซอร์ลงมารอรับตัวอักษรที่บรรทัดที่ 2
}

void setup() {
  lcd.init();
  lcd.backlight();
  resetScreen();
}

void loop() {
  char customKey = customKeypad.getKey();

  if (customKey) {
    if (customKey == '#') {
      resetScreen(); // ถ้ากด # ให้ล้างหน้าจอและตั้งตำแหน่งใหม่
    } else {
      lcd.print(customKey); // ปุ่มอื่น ๆ พิมพ์ลงจอตามปกติ
    }
  }
}