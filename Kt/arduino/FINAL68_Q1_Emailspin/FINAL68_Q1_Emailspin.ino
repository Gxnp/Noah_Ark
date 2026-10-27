// =====================================================================
// Final 68 ข้อ 1 : เครื่องดื่มสูตรพิเศษของเชฟเอียนที่ Clarke Quay (1 คะแนน)
// อ่านค่าตัวต้านทานปรับค่าได้ (Potentiometer) -> publish ไปที่ student_id/emailspin
// เพื่อให้ emailspin ในเว็บ (phycom.it.kmitl.ac.th/exam68) หมุนตามความเร็วที่ส่งไป
//
// บอร์ด Arduino UNO R4 WiFi   ไลบรารี: MQTT (MQTTClient.h)
// วงจร: Potentiometer ขาริม -> 5V และ GND, ขากลาง -> A0
// (ขาเดียวกับไฟล์ FINAL68_ALL_IN_ONE ต่อวงจรครั้งเดียวใช้ได้ทุกข้อ)
// =====================================================================

#include <WiFiS3.h>
#include <MQTTClient.h>

// ===== แก้ตรงนี้ =====
const char WIFI_SSID[]  = "iphone 80 promax";          // hotspot มือถือ 2.4 GHz
const char WIFI_PASSWORD[]  = "88888888";
const String STUDENT_ID = "68070180";           // <<< รหัสนักศึกษา (topic = รหัส/emailspin)

// broker ตามข้อสอบ (บอร์ดใช้พอร์ต 1883 ทุกตัว) เลือก 1 ตัว
const char MQTT_BROKER_ADRRESS[] = "phycom.it.kmitl.ac.th";
// const char MQTT_BROKER_ADRRESS[] = "mqtt-dashboard.com";
// const char MQTT_BROKER_ADRRESS[] = "test.mosquitto.org";
const int   MQTT_PORT = 1883;

// รูปแบบค่าที่ส่ง: false = ค่าดิบ 0–1023, true = เปอร์เซ็นต์ 0–100
// ถ้า emailspin ในเว็บหมุนเร็วสุดตั้งแต่หมุน Pot นิดเดียว ให้เปลี่ยนเป็น true
const bool SEND_PERCENT = false;

const int POT_PIN = A0;

String TOPIC_SPIN;
WiFiClient network;
MQTTClient mqtt = MQTTClient(256);
int lastSent = -100;
unsigned long lastPub = 0;

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
    Serial.print("Connecting MQTT ");
    Serial.print(MQTT_BROKER_ADRRESS);
    Serial.print(" ... ");
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
  TOPIC_SPIN = STUDENT_ID + "/emailspin";
  connectWiFi();
  randomSeed(micros());
  mqtt.begin(MQTT_BROKER_ADRRESS, MQTT_PORT, network);
  connectToMQTT();
}

void loop() {
  if (WiFi.status() != WL_CONNECTED) connectWiFi();
  if (!mqtt.connected()) connectToMQTT();
  mqtt.loop();

  int raw = analogRead(POT_PIN);                        // 0–1023
  int value = SEND_PERCENT ? map(raw, 0, 1023, 0, 100) : raw;
  int threshold = SEND_PERCENT ? 1 : 8;                 // กันค่าแกว่งเล็กน้อย

  // ส่งเมื่อค่าเปลี่ยน หรือทุก 1 วินาที และไม่ถี่กว่า 100 ms
  bool changed = abs(value - lastSent) >= threshold;
  bool timeout = millis() - lastPub >= 1000;
  if ((changed || timeout) && millis() - lastPub >= 100) {
    mqtt.publish(TOPIC_SPIN.c_str(), String(value).c_str());
    Serial.print("[TX] ");
    Serial.print(TOPIC_SPIN);
    Serial.print(" : ");
    Serial.println(value);
    lastSent = value;
    lastPub = millis();
  }
}
