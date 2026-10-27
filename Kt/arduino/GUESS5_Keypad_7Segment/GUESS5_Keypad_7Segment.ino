// แนวข้อสอบ 5 : กดเลข 0–9 บน Keypad -> publish  |  subscribe -> แสดงเลขบน 7-Segment
// กด * = ส่ง "-" ให้ 7-Segment ดับ
// บอร์ด Arduino UNO R4 WiFi   ไลบรารี: MQTT (MQTTClient.h), Adafruit Keypad
// วงจร Keypad: R1–R4 -> D9, D8, D7, D6   C1–C4 -> D5, D4, D3, D2
// วงจร 7-Segment (Common Cathode): ย้ายขาไม่ให้ชน Keypad
//   a -> D10, b -> D11, c -> D12, d -> D13, e -> A0, f -> A1, g -> A2 (ผ่าน R 220Ω)
//   Common -> GND
//   (ขา A0–A5 ใช้เป็นขาดิจิทัลได้)

#include <WiFiS3.h>
#include <MQTTClient.h>
#include "Adafruit_Keypad.h"

// ===== แก้ตรงนี้ =====
const char WIFI_SSID[] = "iphone 80 promax";
const char WIFI_PASSWORD[] = "88888888";
const char MQTT_BROKER_ADRRESS[] = "mqtt-dashboard.com";
const int   MQTT_PORT = 1883;
const char* TOPIC_PUB = "68070180/digit";
const char* TOPIC_SUB = "68070180/digit";

const int SEG_PINS[7] = {10, 11, 12, 13, A0, A1, A2};   // a, b, c, d, e, f, g
const bool COMMON_ANODE = false;

const byte DIGITS[10][7] = {
  {1,1,1,1,1,1,0}, {0,1,1,0,0,0,0}, {1,1,0,1,1,0,1}, {1,1,1,1,0,0,1}, {0,1,1,0,0,1,1},
  {1,0,1,1,0,1,1}, {1,0,1,1,1,1,1}, {1,1,1,0,0,0,0}, {1,1,1,1,1,1,1}, {1,1,1,1,0,1,1}
};

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


WiFiClient network;
MQTTClient mqtt = MQTTClient(256);

void writeSeg(int i, bool on) {
  if (COMMON_ANODE) on = !on;
  digitalWrite(SEG_PINS[i], on ? HIGH : LOW);
}

void showDigit(int n) {
  for (int i = 0; i < 7; i++) writeSeg(i, DIGITS[n][i]);
}

void blank() {
  for (int i = 0; i < 7; i++) writeSeg(i, false);
}

void messageReceived(String &topic, String &payload) {
  (void)topic;
  String msg = payload;
  msg.trim();                                       // ตัดช่องว่างที่พิมพ์เกินมา
  if (msg.length() == 0) return;
  char c = msg[0];
  Serial.print("[RX] ");
  Serial.println(c);
  if (c >= '0' && c <= '9') showDigit(c - '0');
  else blank();
}

void connectWiFi() {
  Serial.print("Connecting WiFi: ");
  Serial.println(WIFI_SSID);
  while (WiFi.begin(WIFI_SSID, WIFI_PASSWORD) != WL_CONNECTED) { Serial.print("."); delay(3000); }
  while (WiFi.localIP() == IPAddress(0, 0, 0, 0)) delay(200);
  Serial.print("WiFi OK, IP = ");
  Serial.println(WiFi.localIP());
}

void connectToMQTT() {
  while (!mqtt.connected()) {
    String clientId = "r4-g5-" + String(random(0xFFFFFF), HEX);
    if (mqtt.connect(clientId.c_str())) {
      mqtt.subscribe(TOPIC_SUB);
      Serial.println("MQTT OK");
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
  for (int i = 0; i < 7; i++) pinMode(SEG_PINS[i], OUTPUT);
  blank();
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
    char payload[2] = {key, '\0'};
    mqtt.publish(TOPIC_PUB, payload);
    Serial.print("[TX] ");
    Serial.println(payload);
  } else if (key == '*') {
    mqtt.publish(TOPIC_PUB, "-");
  }
}
