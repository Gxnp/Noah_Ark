// แนวข้อสอบ 3 : กดเลข 0–7 บน Keypad เลือกสี -> publish  |  subscribe -> RGB แสดงสี + ชื่อสีบน LCD
// 0 = ดับ 1 = แดง 2 = เขียว 3 = น้ำเงิน 4 = เหลือง 5 = ฟ้า 6 = ม่วง 7 = ขาว
// บอร์ด Arduino UNO R4 WiFi   ไลบรารี: MQTT (MQTTClient.h), Adafruit Keypad, LiquidCrystal_I2C
// วงจร Keypad: R1–R4 -> D9, D8, D7, D6   C1–C4 -> D5, D4, D3, D2
// วงจร RGB (Common Anode): Common -> 5V, R -> D10, G -> D11, B -> D12 (ผ่าน R 220Ω)
//   ย้ายขา RGB จากแลป 12.2 เพราะ D9 ถูก Keypad ใช้แล้ว
// วงจร LCD: SDA -> SDA, SCL -> SCL, VCC -> 5V, GND -> GND

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
const char* TOPIC_PUB = "68070180/color";
const char* TOPIC_SUB = "68070180/color";

const int R_PIN = 10, G_PIN = 11, B_PIN = 12;
const bool COMMON_ANODE = true;

const int COLORS[8][3] = {
  {0, 0, 0}, {1, 0, 0}, {0, 1, 0}, {0, 0, 1},
  {1, 1, 0}, {0, 1, 1}, {1, 0, 1}, {1, 1, 1}
};
const char* NAMES[8] = {"OFF", "RED", "GREEN", "BLUE", "YELLOW", "CYAN", "MAGENTA", "WHITE"};

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

void lcdLine(int row, String s) {
  lcd.setCursor(0, row);
  while (s.length() < 16) s += ' ';
  lcd.print(s.substring(0, 16));
}

void setRgb(int r, int g, int b) {
  if (COMMON_ANODE) { r = !r; g = !g; b = !b; }
  digitalWrite(R_PIN, r);
  digitalWrite(G_PIN, g);
  digitalWrite(B_PIN, b);
}

void messageReceived(String &topic, String &payload) {
  (void)topic;
  String msg = payload;

  int n = msg.toInt();
  if (n < 0 || n > 7) return;              // ไม่ใช่ 0–7 ไม่ต้องทำอะไร
  setRgb(COLORS[n][0], COLORS[n][1], COLORS[n][2]);
  lcdLine(1, "Color: " + String(NAMES[n]));
  Serial.print("[RX] ");
  Serial.println(NAMES[n]);
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
    String clientId = "r4-g3-" + String(random(0xFFFFFF), HEX);
    if (mqtt.connect(clientId.c_str())) {
      mqtt.subscribe(TOPIC_SUB);
      lcdLine(0, "Press 0-7");
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
  pinMode(R_PIN, OUTPUT);
  pinMode(G_PIN, OUTPUT);
  pinMode(B_PIN, OUTPUT);
  setRgb(0, 0, 0);
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
  if (key >= '0' && key <= '7') {
    char payload[2] = {key, '\0'};
    mqtt.publish(TOPIC_PUB, payload);
    lcdLine(0, "TX: " + String(key) + " " + NAMES[key - '0']);
    Serial.print("[TX] ");
    Serial.println(payload);
  }
}
