// =====================================================================
// สไลด์ PC69-14 แบบฝึกหัดที่ 1 + 2 : Digital thermometer + Custom Font
//   แถวแรก : "I ♥ IT" อยู่กลางแถว (♥ เป็น custom character)
//   แถวสอง : รหัสนักศึกษา และอุณหภูมิทศนิยม 2 ตำแหน่ง เช่น "68070180 25.54°"
//            (° เป็น custom character)
//
// บอร์ด Arduino UNO R4 WiFi   ไลบรารี: LiquidCrystal_I2C
// วงจร MCP9700: VDD -> 5V, Vout -> A0, GND -> GND (ตามสไลด์ใช้ A0)
//      LCD I2C: GND -> GND, VCC -> 5V, SDA -> A4, SCL -> A5
// =====================================================================

#include <Wire.h>
#include <LiquidCrystal_I2C.h>

const char STUDENT_ID[] = "68070180";
const int TEMP_PIN = A0;

LiquidCrystal_I2C lcd(0x27, 16, 2);   // ถ้าจอไม่ขึ้น ลอง 0x3F

byte heartChar[8] = {                  // ♥  (สไลด์เขียน B01010 ก็ได้ แต่ IDE ใหม่เตือนว่าล้าสมัย)
  0b00000,
  0b01010,
  0b11111,
  0b11111,
  0b11111,
  0b01110,
  0b00100,
  0b00000
};

byte degreeChar[8] = {                 // °
  0b00110,
  0b01001,
  0b01001,
  0b00110,
  0b00000,
  0b00000,
  0b00000,
  0b00000
};

float readTempC() {
  // สไลด์: Temp = 100 * ((Val * 5) / 1023 - 1) + 50
  long sum = 0;
  for (int i = 0; i < 10; i++) { sum += analogRead(TEMP_PIN); delay(2); }
  float val = sum / 10.0;
  return 100.0 * ((val * 5.0) / 1023.0 - 1.0) + 50.0;
}

void setup() {
  Serial.begin(9600);
  lcd.init();
  lcd.backlight();
  lcd.createChar(1, heartChar);
  lcd.createChar(2, degreeChar);

  // แถวแรก: "I ♥ IT" ยาว 6 ตัว -> เริ่มคอลัมน์ (16 - 6) / 2 = 5 จะอยู่กลางพอดี
  lcd.setCursor(5, 0);
  lcd.print("I ");
  lcd.write(byte(1));
  lcd.print(" IT");
}

void loop() {
  float t = readTempC();

  // แถวสอง: "68070180 25.54°" = 15 ตัว
  lcd.setCursor(0, 1);
  lcd.print(STUDENT_ID);
  lcd.print(" ");
  lcd.print(t, 2);                     // ทศนิยม 2 ตำแหน่ง
  lcd.write(byte(2));
  lcd.print("  ");                     // ลบตัวเลขเก่าที่อาจยาวกว่า

  Serial.print("Temp = ");
  Serial.println(t, 2);
  delay(500);
}
