// =====================================================================
// Final 68 ข้อ 2 : 7-segment กับการสุ่ม Gate ที่ Changi (1 คะแนน)
// subscribe topic student_id/prob_stat (ตัวเลข 0–9 ที่ระบบสุ่มส่งมา)
// แล้วแสดงบน 7-segment ขนาด 1 หลัก
//
// บอร์ด Arduino UNO R4 WiFi   ไลบรารี: MQTT (MQTTClient.h)
// วงจร 7-Segment (Common Cathode):
//   a -> D2, b -> D3, c -> D4, d -> D5, e -> D6, f -> D7, g -> D8 (ผ่าน R 330Ω ทุกเส้น)
//   Common -> GND
// =====================================================================

#include <WiFiS3.h>
#include <MQTTClient.h>

// ===== แก้ตรงนี้ =====
const char WIFI_SSID[]  = "iphone 80 promax";
const char WIFI_PASSWORD[]  = "88888888";
const String STUDENT_ID = "68070180";           // <<< รหัสนักศึกษา

const char MQTT_BROKER_ADRRESS[] = "phycom.it.kmitl.ac.th";
// const char MQTT_BROKER_ADRRESS[] = "mqtt-dashboard.com";
// const char MQTT_BROKER_ADRRESS[] = "test.mosquitto.org";
const int   MQTT_PORT = 1883;

const int SEG_PINS[7] = {2, 3, 4, 5, 6, 7, 8};  // a, b, c, d, e, f, g
const bool COMMON_ANODE = false;                 // ถ้า 7-Segment เป็น Common Anode ให้เป็น true และต่อ Common -> 5V

//          a
//        -----
//     f |     | b
//        --g--
//     e |     | c
//        -----
//          d
const byte DIGITS[10][7] = {
  {1,1,1,1,1,1,0},  // 0
  {0,1,1,0,0,0,0},  // 1
  {1,1,0,1,1,0,1},  // 2
  {1,1,1,1,0,0,1},  // 3
  {0,1,1,0,0,1,1},  // 4
  {1,0,1,1,0,1,1},  // 5
  {1,0,1,1,1,1,1},  // 6
  {1,1,1,0,0,0,0},  // 7
  {1,1,1,1,1,1,1},  // 8
  {1,1,1,1,0,1,1}   // 9
};

String TOPIC_GATE;
WiFiClient network;
MQTTClient mqtt = MQTTClient(256);

void writeSeg(int i, bool on) {
  if (COMMON_ANODE) on = !on;
  digitalWrite(SEG_PINS[i], on ? HIGH : LOW);
}

void showDigit(int n) {
  for (int i = 0; i < 7; i++) writeSeg(i, DIGITS[n][i]);
}

void showDash() {                                // ขีดกลาง = ยังไม่ได้รับค่า / ค่าผิด
  for (int i = 0; i < 7; i++) writeSeg(i, i == 6);
}

void messageReceived(String &topic, String &payload) {
  String msg = payload;
  msg.trim();
  Serial.print("[RX] ");
  Serial.print(topic);
  Serial.print(" : ");
  Serial.println(msg);

  // รับได้ทั้ง "7", "7.0" หรือ "Gate 7" โดยหาตัวเลขตัวแรกในข้อความ
  for (unsigned int i = 0; i < msg.length(); i++) {
    if (isDigit(msg[i])) { showDigit(msg[i] - '0'); return; }
  }
  showDash();
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
    String clientId = "r4-" + STUDENT_ID + "-" + String(random(0xFFFF), HEX);
    Serial.print("Connecting MQTT ... ");
    if (mqtt.connect(clientId.c_str())) {
      Serial.println("OK");
      mqtt.subscribe(TOPIC_GATE.c_str());
      Serial.println("sub " + TOPIC_GATE);
    } else {
      Serial.print("fail err=");
      Serial.print(mqtt.lastError()); Serial.print(" rc="); Serial.println(mqtt.returnCode());
      delay(2000);
    }
  }
}

void setup() {
  Serial.begin(9600);
  for (int i = 0; i < 7; i++) pinMode(SEG_PINS[i], OUTPUT);
  showDash();
  TOPIC_GATE = STUDENT_ID + "/prob_stat";
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
}
