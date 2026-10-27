// งาน 15.2 : MQTT Keypad — ฝั่งคนส่ง (Publisher)
// กด Keypad -> publish ค่าปุ่มขึ้น HiveMQ -> บอร์ดเพื่อน (W15_2_MQTT_LCD_Receiver) แสดงบนจอ LCD
// บอร์ด Arduino UNO R4 WiFi   ไลบรารี: MQTT (MQTTClient.h), Adafruit Keypad
// วงจร Keypad 4x4 (มองจากด้านหน้า ขาเรียงจากซ้าย):
//   R1 -> D9, R2 -> D8, R3 -> D7, R4 -> D6, C1 -> D5, C2 -> D4, C3 -> D3, C4 -> D2

#include <WiFiS3.h>
#include <MQTTClient.h>
#include "Adafruit_Keypad.h"

// ===== แก้ตรงนี้ (topic ต้องเหมือนกับฝั่งคนรับ) =====
const char WIFI_SSID[] = "iphone 80 promax";
const char WIFI_PASSWORD[] = "88888888";
const char MQTT_BROKER_ADRRESS[] = "mqtt-dashboard.com";
const int   MQTT_PORT = 1883;
const char* TOPIC_PUB = "68070180/keypad";   // ตั้งชื่อกลุ่มให้ไม่ซ้ำคู่อื่น

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

void connectWiFi() {
  Serial.print("Connecting WiFi: ");
  Serial.println(WIFI_SSID);
  while (WiFi.begin(WIFI_SSID, WIFI_PASSWORD) != WL_CONNECTED) {
    Serial.print(".");
    delay(3000);
  }
  while (WiFi.localIP() == IPAddress(0, 0, 0, 0)) delay(200);
  Serial.print("WiFi OK, IP = ");
  Serial.println(WiFi.localIP());
}

void connectToMQTT() {
  while (!mqtt.connected()) {
    String clientId = "r4-kp-" + String(random(0xFFFFFF), HEX);
    Serial.print("Connecting MQTT ... ");
    if (mqtt.connect(clientId.c_str())) {
      Serial.println("OK");
    } else {
      Serial.print("fail err=");
      Serial.print(mqtt.lastError()); Serial.print(" rc="); Serial.println(mqtt.returnCode());
      delay(2000);
    }
  }
}

void setup() {
  Serial.begin(9600);
  keypad.begin();
  connectWiFi();
  randomSeed(micros());
  mqtt.begin(MQTT_BROKER_ADRRESS, MQTT_PORT, network);
  connectToMQTT();
  Serial.println("พร้อม กด Keypad ได้เลย");
}

void loop() {
  if (WiFi.status() != WL_CONNECTED) connectWiFi();
  if (!mqtt.connected()) connectToMQTT();
  mqtt.loop();

  char key = getKey();          // ไม่มีการกดจะได้ 0 (NO_KEY)
  if (key) {
    char payload[2] = {key, '\0'};     // แปลงตัวอักษรเป็นข้อความ 1 ตัว
    mqtt.publish(TOPIC_PUB, payload);
    Serial.print("[TX] ");
    Serial.print(TOPIC_PUB);
    Serial.print(" : ");
    Serial.println(payload);
  }
}
