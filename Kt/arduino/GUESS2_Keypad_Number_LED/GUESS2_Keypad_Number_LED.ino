// แนวข้อสอบ 2 : พิมพ์ตัวเลขบน Keypad แล้วกด # ส่ง -> publish
//               subscribe -> ปรับความสว่าง LED ตามตัวเลข (0–255) และแสดงบน LCD
// กด * = ลบตัวเลขที่พิมพ์   กด A = ส่ง 255 (สว่างสุด)   กด B = ส่ง 0 (ดับ)
// บอร์ด Arduino UNO R4 WiFi   ไลบรารี: MQTT (MQTTClient.h), Adafruit Keypad, LiquidCrystal_I2C
// วงจร Keypad: R1–R4 -> D9, D8, D7, D6   C1–C4 -> D5, D4, D3, D2
// วงจร LED  : D10 (PWM ~) -> R 220Ω -> LED -> GND
// วงจร LCD  : SDA -> SDA, SCL -> SCL, VCC -> 5V, GND -> GND

#include <WiFiS3.h>
#include <MQTTClient.h>
#include "Adafruit_Keypad.h"
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

// ===== แก้ตรงนี้ =====
const char WIFI_SSID[] = "iphone 80 promax";
const char WIFI_PASSWORD[] = "88888888";
const char MQTT_BROKER_ADRRESS[] = "mqtt-dashboard.com";
const int   MQTT_PORT = 1883;
const char* TOPIC_PUB = "68070180/brightness";
const char* TOPIC_SUB = "68070180/brightness";

const int LED_PIN = 10;    // ขา PWM ที่ว่างอยู่ (3, 5, 6, 9 ถูก Keypad ใช้)

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
// อ่านปุ่ม Keypad แบบไลบรารี Adafruit_Keypad (แบบเดียวกับโค้ดของวิชา) คืน 0 ถ้าไม่มีการกด
char getKey() {
  keypad.tick();
  while (keypad.available()) {
    keypadEvent e = keypad.read();
    if (e.bit.EVENT == KEY_JUST_PRESSED) return (char)e.bit.KEY;
  }
  return 0;
}


LiquidCrystal_I2C lcd(0x27, 16, 2);
WiFiClient network;
MQTTClient mqtt = MQTTClient(256);
String input = "";        // ตัวเลขที่กำลังพิมพ์

void lcdLine(int row, String s) {
  lcd.setCursor(0, row);
  while (s.length() < 16) s += ' ';
  lcd.print(s.substring(0, 16));
}

void sendValue(String v) {
  mqtt.publish(TOPIC_PUB, v.c_str());
  Serial.print("[TX] ");
  Serial.println(v);
}

void messageReceived(String &topic, String &payload) {
  (void)topic;
  String msg = payload;

  int v = constrain(msg.toInt(), 0, 255);
  analogWrite(LED_PIN, v);
  int percent = map(v, 0, 255, 0, 100);
  lcdLine(1, "LED:" + String(v) + " (" + String(percent) + "%)");
  Serial.print("[RX] ");
  Serial.println(v);
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
    String clientId = "r4-g2-" + String(random(0xFFFFFF), HEX);
    if (mqtt.connect(clientId.c_str())) {
      mqtt.subscribe(TOPIC_SUB);
      lcdLine(0, "In: " + input);
    } else {
      Serial.print("MQTT fail err=");
      Serial.print(mqtt.lastError()); Serial.print(" rc="); Serial.println(mqtt.returnCode());
      delay(2000);
    }
  }
}

void setup() {
  Serial.begin(9600);
  keypad.begin();
  pinMode(LED_PIN, OUTPUT);
  lcd.init();
  lcd.backlight();
  connectWiFi();
  randomSeed(micros());
  mqtt.begin(MQTT_BROKER_ADRRESS, MQTT_PORT, network);
  mqtt.onMessage(messageReceived);
  connectToMQTT();
}

void loop() {
  if (WiFi.status() != WL_CONNECTED) connectWiFi();
  if (!mqtt.connected()) connectToMQTT();
  mqtt.loop();

  char key = getKey();
  if (!key) return;

  if (key >= '0' && key <= '9') {
    if (input.length() < 3) input += key;   // รับไม่เกิน 3 หลัก
  } else if (key == '*') {
    input = "";
  } else if (key == '#') {
    if (input.length() > 0) {
      int v = constrain(input.toInt(), 0, 255);
      sendValue(String(v));
      input = "";
    }
  } else if (key == 'A') {
    sendValue("255");
  } else if (key == 'B') {
    sendValue("0");
  }
  lcdLine(0, "In: " + input);
}
