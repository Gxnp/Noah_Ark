// =====================================================================
// แนวข้อสอบ The Shining  S1 : เครื่องพิมพ์ดีดของ Jack (เดา · แบบเดียวกับ Final 68 ข้อ 1)
// "All work and no play makes Jack a dull boy"
// หมุน Potentiometer = ความเร็วการพิมพ์ -> publish ไปที่ student_id/typewriter
// และให้ LCD เลื่อนประโยคนั้นด้วยความเร็วตาม Pot (ยิ่งหมุนมาก ยิ่งพิมพ์เร็ว)
//
// บอร์ด Arduino UNO R4 WiFi   ไลบรารี: MQTT (MQTTClient.h), LiquidCrystal_I2C
// วงจร: Potentiometer ขากลาง -> A0 (ขาริม 5V, GND)
//       LCD I2C: SDA, SCL, VCC -> 5V, GND
// =====================================================================

#include <WiFiS3.h>
#include <MQTTClient.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

// ===== แก้ตรงนี้ =====
const char WIFI_SSID[]  = "iphone 80 promax";
const char WIFI_PASSWORD[]  = "88888888";
const String STUDENT_ID = "68070180";
const char MQTT_BROKER_ADRRESS[] = "phycom.it.kmitl.ac.th";     // หรือ mqtt-dashboard.com / test.mosquitto.org
const int   MQTT_PORT = 1883;
const String TOPIC_NAME = "/typewriter";              // <<< เปลี่ยนตามโจทย์จริง
const bool SEND_PERCENT = false;                      // false = 0–1023, true = 0–100

const int POT_PIN = A0;
const String LINE = "All work and no play makes Jack a dull boy. ";

LiquidCrystal_I2C lcd(0x27, 16, 2);
WiFiClient network;
MQTTClient mqtt = MQTTClient(256);
String TOPIC;
int lastSent = -100;
unsigned long lastPub = 0, lastType = 0;
unsigned int typed = 0;           // พิมพ์ไปกี่ตัวแล้ว

void lcdLine(int row, String s) {
  lcd.setCursor(0, row);
  while (s.length() < 16) s += ' ';
  lcd.print(s.substring(0, 16));
}

void connectWiFi() {
  lcdLine(0, "WiFi...");
  while (WiFi.begin(WIFI_SSID, WIFI_PASSWORD) != WL_CONNECTED) { Serial.print("."); delay(3000); }
  while (WiFi.localIP() == IPAddress(0, 0, 0, 0)) delay(200);
  Serial.print("WiFi OK, IP = ");
  Serial.println(WiFi.localIP());
}

void connectToMQTT() {
  while (!mqtt.connected()) {
    lcdLine(0, "MQTT...");
    String id = "r4-" + STUDENT_ID + "-" + String(random(0xFFFF), HEX);
    if (mqtt.connect(id.c_str())) { Serial.println("MQTT OK"); lcdLine(0, "Jack is typing"); }
    else { Serial.print("MQTT fail err="); Serial.print(mqtt.lastError()); Serial.print(" rc="); Serial.println(mqtt.returnCode()); delay(2000); }
  }
}

void setup() {
  Serial.begin(9600);
  lcd.init();
  lcd.backlight();
  TOPIC = STUDENT_ID + TOPIC_NAME;
  connectWiFi();
  randomSeed(micros());
  mqtt.begin(MQTT_BROKER_ADRRESS, MQTT_PORT, network);
  connectToMQTT();
}

void loop() {
  if (WiFi.status() != WL_CONNECTED) connectWiFi();
  if (!mqtt.connected()) connectToMQTT();
  mqtt.loop();

  int raw = analogRead(POT_PIN);
  int value = SEND_PERCENT ? map(raw, 0, 1023, 0, 100) : raw;

  // ---- publish ความเร็ว ----
  bool changed = abs(value - lastSent) >= (SEND_PERCENT ? 1 : 8);
  if ((changed || millis() - lastPub >= 1000) && millis() - lastPub >= 100) {
    mqtt.publish(TOPIC.c_str(), String(value).c_str());
    Serial.println("[TX] " + TOPIC + " : " + String(value));
    lastSent = value;
    lastPub = millis();
  }

  // ---- LCD พิมพ์ทีละตัว: Pot 0 = หยุด, Pot สูงสุด = 20 ตัว/วินาที ----
  if (raw < 20) return;                               // หมุนสุดซ้าย = Jack หยุดพิมพ์
  unsigned long interval = map(raw, 20, 1023, 600, 50);
  if (millis() - lastType >= interval) {
    lastType = millis();
    typed++;
    String loopText = LINE + LINE;
    unsigned int start = typed % LINE.length();
    lcdLine(1, loopText.substring(start, start + 16));
  }
}
