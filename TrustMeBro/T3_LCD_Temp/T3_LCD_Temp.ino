#include <LiquidCrystal_I2C.h>
LiquidCrystal_I2C lcd(0x27, 16, 2);

void setup() {
  lcd.init();
  lcd.backlight();
}

void loop() {
  int sensorVal = analogRead(A0);
  
  float voltage = (sensorVal * 5.0) / 1024.0;
  
  float tempC = (voltage - 0.5) * 100.0;
  
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Temp: ");
  // lcd.setCursor(0, 1); //ขึ้นบรรทัดใหม่ (ลำดับ,บรรทัด)
  lcd.print(tempC, 2);
  // lcd.print(tempC, 0);  // 29
  // lcd.print(tempC, 1);  // 28.6
  // lcd.print(tempC, 2);  // 28.57
  // lcd.print(tempC, 3);  // 28.567
  lcd.print(" C");
  
  delay(1000);
}