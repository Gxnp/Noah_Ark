// =====================================================================
// แนวข้อสอบ The Shining  S2 : ห้อง 237 (เดา · แบบเดียวกับ Final 68 ข้อ 2)
// subscribe student_id/room แล้วแสดงบน 7-Segment 1 หลัก
//   ได้ตัวเลขหลักเดียว เช่น "7"  -> แสดง 7 ค้างไว้
//   ได้หลายหลัก เช่น "237"       -> แสดง 2 → 3 → 7 วนทีละหลัก หลักละ 1 วินาที
//   ได้ข้อความไม่มีตัวเลข        -> แสดงขีดกลาง
//
// บอร์ด Arduino UNO R4 WiFi   ไลบรารี: MQTT (MQTTClient.h)
// วงจร 7-Segment (Common Cathode): a–g -> D2–D8 (ผ่าน R 330Ω), Common -> GND
// =====================================================================

#include <WiFiS3.h>
#include <MQTTClient.h>

// ===== แก้ตรงนี้ =====
const char WIFI_SSID[]  = "iphone 80 promax";
const char WIFI_PASSWORD[]  = "88888888";
const String STUDENT_ID = "68070180";
const char MQTT_BROKER_ADRRESS[] = "phycom.it.kmitl.ac.th";
const int   MQTT_PORT = 1883;
const String TOPIC_NAME = "/room";                    // <<< เปลี่ยนตามโจทย์จริง

const int SEG_PINS[7] = {2, 3, 4, 5, 6, 7, 8};        // a b c d e f g
const bool COMMON_ANODE = false;
const byte DIGITS[10][7] = {
  {1,1,1,1,1,1,0}, {0,1,1,0,0,0,0}, {1,1,0,1,1,0,1}, {1,1,1,1,0,0,1}, {0,1,1,0,0,1,1},
  {1,0,1,1,0,1,1}, {1,0,1,1,1,1,1}, {1,1,1,0,0,0,0}, {1,1,1,1,1,1,1}, {1,1,1,1,0,1,1}
};

WiFiClient network;
MQTTClient mqtt = MQTTClient(256);
String TOPIC;
String digits = "";              // ตัวเลขที่ต้องแสดง เช่น "237"
unsigned int pos = 0;
unsigned long lastStep = 0;

void writeSeg(int i, bool on) { if (COMMON_ANODE) on = !on; digitalWrite(SEG_PINS[i], on ? HIGH : LOW); }
void showDigit(int n) { for (int i = 0; i < 7; i++) writeSeg(i, DIGITS[n][i]); }
void showDash()       { for (int i = 0; i < 7; i++) writeSeg(i, i == 6); }
void blank()          { for (int i = 0; i < 7; i++) writeSeg(i, false); }

void messageReceived(String &topic, String &payload) {
  String msg = payload;
  Serial.print("[RX] ");
  Serial.print(topic);
  Serial.print(" : ");
  Serial.println(msg);

  digits = "";
  for (unsigned int i = 0; i < msg.length(); i++) if (isDigit(msg[i])) digits += msg[i];   // เก็บเฉพาะตัวเลข
  pos = 0;
  lastStep = millis();
  if (digits.length() == 0) showDash();
  else showDigit(digits[0] - '0');
}

void connectWiFi() {
  while (WiFi.begin(WIFI_SSID, WIFI_PASSWORD) != WL_CONNECTED) { Serial.print("."); delay(3000); }
  while (WiFi.localIP() == IPAddress(0, 0, 0, 0)) delay(200);
  Serial.print("WiFi OK, IP = ");
  Serial.println(WiFi.localIP());
}

void connectToMQTT() {
  while (!mqtt.connected()) {
    String id = "r4-" + STUDENT_ID + "-" + String(random(0xFFFF), HEX);
    if (mqtt.connect(id.c_str())) { mqtt.subscribe(TOPIC.c_str()); Serial.println("MQTT OK, sub " + TOPIC); }
    else { Serial.print("MQTT fail err="); Serial.print(mqtt.lastError()); Serial.print(" rc="); Serial.println(mqtt.returnCode()); delay(2000); }
  }
}

void setup() {
  Serial.begin(9600);
  for (int i = 0; i < 7; i++) pinMode(SEG_PINS[i], OUTPUT);
  showDash();
  TOPIC = STUDENT_ID + TOPIC_NAME;
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

  // หลายหลัก: เลื่อนทีละหลักทุก 1 วินาที (ดับสั้น ๆ ระหว่างหลัก ให้เห็นเลขซ้ำ เช่น 2-2)
  if (digits.length() > 1 && millis() - lastStep >= 1000) {
    lastStep = millis();
    pos = (pos + 1) % digits.length();
    blank();
    delay(120);
    showDigit(digits[pos] - '0');
  }
}
