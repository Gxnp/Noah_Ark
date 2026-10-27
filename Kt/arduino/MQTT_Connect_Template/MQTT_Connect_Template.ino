// =====================================================================
// แม่แบบเชื่อมต่อ MQTT สำหรับ Arduino UNO R4 WiFi (ใช้เริ่มทุกข้อ)
// - เลือก broker ด้วย BROKER = 1 / 2 / 3 (ตามตารางในข้อสอบ บอร์ดใช้พอร์ต 1883 ทุกตัว)
// - topic เป็นรูปแบบ "รหัสนักศึกษา/ชื่อ" เช่น 68070180/temp
// - ทดสอบ: บอร์ดส่ง "hello <เวลา>" ไปที่ รหัส/test ทุก 2 วินาที
//          และ echo ทุกข้อความที่ส่งเข้า รหัส/cmd กลับไปที่ รหัส/echo
//
// ไลบรารี: MQTT (Joel Gaehwiler) → MQTTClient.h   |   WiFiS3 มากับบอร์ด R4 แล้ว
// =====================================================================

#include <WiFiS3.h>
#include <MQTTClient.h>

// ---------- 1) WiFi ----------
const char WIFI_SSID[] = "iphone 80 promax";             // hotspot มือถือ 2.4 GHz ชื่อต้องตรงทุกตัวอักษร (เว้นวรรคได้)
const char WIFI_PASSWORD[] = "88888888";

// ---------- 2) Broker ----------
//  1 = phycom.it.kmitl.ac.th   (เว็บ: phycom.it.kmitl.ac.th/mqtt  พอร์ต 8884)
//  2 = mqtt-dashboard.com      (HiveMQ public = mqtt-dashboard.com  เว็บพอร์ต 8884)
//  3 = test.mosquitto.org      (เว็บพอร์ต 8081)
const int BROKER = 1;
const char* HOSTS[] = {"", "phycom.it.kmitl.ac.th", "mqtt-dashboard.com", "test.mosquitto.org"};
const int   MQTT_PORT = 1883;                    // บอร์ดใช้ 1883 เสมอ (MQTT ผ่าน TCP)

// ---------- 3) Topic ----------
const String STUDENT_ID = "68070180";            // <<< รหัสนักศึกษา
String T_TEST, T_CMD, T_ECHO;

WiFiClient network;                           // 1883 ไม่เข้ารหัส ใช้ WiFiClient
MQTTClient mqtt = MQTTClient(512);
unsigned long lastPub = 0;
String pendingEcho = "";

// ---------- รับข้อความ ----------
void messageReceived(String &topic, String &payload) {
  String msg = payload;
  msg.trim();
  Serial.print("[RX] ");
  Serial.print(topic);
  Serial.print(" : ");
  Serial.println(msg);

  pendingEcho = msg;      // ห้าม publish ใน callback (ไลบรารีอาจค้าง) เก็บไว้ส่งใน loop()
}

// ---------- ต่อ WiFi ----------
void connectWiFi() {
  Serial.print("Connecting WiFi: ");
  Serial.println(WIFI_SSID);
  while (WiFi.begin(WIFI_SSID, WIFI_PASSWORD) != WL_CONNECTED) {
    Serial.print(".");
    delay(3000);
  }
  while (WiFi.localIP() == IPAddress(0, 0, 0, 0)) delay(200);   // รอ IP
  Serial.print("WiFi OK, IP = ");
  Serial.println(WiFi.localIP());
}

// ---------- ต่อ MQTT ----------
void connectToMQTT() {
  while (!mqtt.connected()) {
    String clientId = "r4-" + STUDENT_ID + "-" + String(random(0xFFFF), HEX);   // ห้ามซ้ำ
    Serial.print("Connecting MQTT ");
    Serial.print(HOSTS[BROKER]);
    Serial.print(":");
    Serial.print(MQTT_PORT);
    Serial.print(" ... ");
    if (mqtt.connect(clientId.c_str())) {
      // ถ้า broker ต้องใช้รหัส: mqtt.connect(clientId.c_str(), "user", "pass")
      Serial.println("OK");
      mqtt.subscribe(T_CMD.c_str());             // subscribe ทุกครั้งหลังต่อติด
    } else {
      // -4 timeout  -2 ต่อไม่ได้ (host/เน็ต)  -3 หลุด  4/5 user/pass ผิด
      Serial.print("fail err=");
      Serial.print(mqtt.lastError()); Serial.print(" rc="); Serial.println(mqtt.returnCode());
      delay(2000);
    }
  }
}

void setup() {
  Serial.begin(9600);
  T_TEST = STUDENT_ID + "/test";
  T_CMD  = STUDENT_ID + "/cmd";
  T_ECHO = STUDENT_ID + "/echo";

  connectWiFi();
  randomSeed(micros());
  mqtt.begin(HOSTS[BROKER], MQTT_PORT, network);
  mqtt.onMessage(messageReceived);
  connectToMQTT();
}

void loop() {
  if (WiFi.status() != WL_CONNECTED) connectWiFi();
  if (!mqtt.connected()) connectToMQTT();
  mqtt.loop();                                   // ต้องเรียกทุกรอบ ห้าม delay นาน

  if (pendingEcho.length() > 0) {                // ส่ง echo หลัง loop()
    mqtt.publish(T_ECHO.c_str(), pendingEcho.c_str());
    Serial.println("[TX] " + T_ECHO + " : " + pendingEcho);
    pendingEcho = "";
  }

  if (millis() - lastPub >= 2000) {
    lastPub = millis();
    String msg = "hello " + String(millis() / 1000);
    mqtt.publish(T_TEST.c_str(), msg.c_str());
    Serial.println("[TX] " + T_TEST + " : " + msg);
  }
}
