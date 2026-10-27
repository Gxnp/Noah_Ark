// แนวข้อสอบ 6 : อ่าน Potentiometer -> publish เป็นเปอร์เซ็นต์ 0–100
//               subscribe -> แสดงตัวเลขบน LED Matrix 12x8 ที่อยู่บนบอร์ด R4 (จากแลป 12)
// บอร์ด Arduino UNO R4 WiFi
// ไลบรารี: MQTT (MQTTClient.h), ArduinoGraphics (ต้องติดตั้งเพิ่ม)  |  Arduino_LED_Matrix มากับบอร์ดแล้ว
// วงจร: Potentiometer ขาริม -> 5V และ GND, ขากลาง -> A0  (LED Matrix อยู่บนบอร์ด ไม่ต้องต่อ)

#include <WiFiS3.h>
#include <MQTTClient.h>
#include "ArduinoGraphics.h"        // ต้อง include ก่อน Arduino_LED_Matrix
#include "Arduino_LED_Matrix.h"

// ===== แก้ตรงนี้ =====
const char WIFI_SSID[] = "iphone 80 promax";
const char WIFI_PASSWORD[] = "88888888";
const char MQTT_BROKER_ADRRESS[] = "mqtt-dashboard.com";
const int   MQTT_PORT = 1883;
const char* TOPIC_PUB = "68070180/percent";
const char* TOPIC_SUB = "68070180/percent";

const int POT_PIN = A0;

ArduinoLEDMatrix matrix;
WiFiClient network;
MQTTClient mqtt = MQTTClient(256);
unsigned long lastPub = 0;
int lastSent = -1;

// ข้อความไม่เกิน 3 ตัว (เช่น 0–100) แสดงนิ่ง ถ้ายาวกว่านั้นให้เลื่อน
void showOnMatrix(String text) {
  matrix.beginDraw();
  matrix.clear();
  matrix.stroke(0xFFFFFFFF);
  matrix.textFont(Font_4x6);
  if (text.length() <= 3) {
    int x = (12 - (int)text.length() * 4) / 2;   // จัดกึ่งกลาง (กว้าง 12 จุด ตัวละ 4 จุด)
    matrix.beginText(x, 1, 0xFFFFFF);
    matrix.print(text);
    matrix.endText();
  } else {
    matrix.textScrollSpeed(60);
    matrix.beginText(0, 1, 0xFFFFFF);
    matrix.print("  " + text);
    matrix.endText(SCROLL_LEFT);
  }
  matrix.endDraw();
}

void messageReceived(String &topic, String &payload) {
  (void)topic;
  String msg = payload;
  Serial.print("[RX] ");
  Serial.println(msg);
  showOnMatrix(msg);
}

void connectWiFi() {
  showOnMatrix("W");
  while (WiFi.begin(WIFI_SSID, WIFI_PASSWORD) != WL_CONNECTED) { Serial.print("."); delay(3000); }
  while (WiFi.localIP() == IPAddress(0, 0, 0, 0)) delay(200);
  Serial.print("WiFi OK, IP = ");
  Serial.println(WiFi.localIP());
}

void connectToMQTT() {
  while (!mqtt.connected()) {
    showOnMatrix("M");
    String clientId = "r4-g6-" + String(random(0xFFFFFF), HEX);
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
  matrix.begin();
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

  if (millis() - lastPub >= 500) {
    lastPub = millis();
    int percent = map(analogRead(POT_PIN), 0, 1023, 0, 100);
    if (percent != lastSent) {                        // ส่งเฉพาะตอนค่าเปลี่ยน
      mqtt.publish(TOPIC_PUB, String(percent).c_str());
      Serial.print("[TX] ");
      Serial.println(percent);
      lastSent = percent;
    }
  }
}
