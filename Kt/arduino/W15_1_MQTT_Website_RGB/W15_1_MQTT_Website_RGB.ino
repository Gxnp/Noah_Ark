// งาน 15.1 : MQTT Website ควบคุมการผสมสี Color LED (RGB)
// หน้าเว็บ (control.html) publish ผ่าน WebSocket -> HiveMQ -> บอร์ด subscribe -> เปิด/ปิดแต่ละสี
// เช่น Red: ON, Green: OFF, Blue: ON -> สีม่วง
// บอร์ด Arduino UNO R4 WiFi   ไลบรารี: MQTT (Joel Gaehwiler) → MQTTClient.h  |  WiFiS3 มากับบอร์ดแล้ว
// วงจร RGB Common Anode: ขา Common -> 5V, R -> D9, B -> D10, G -> D11 (ผ่าน R 220Ω)
//
// Topic ที่ใช้ (ต้องตรงกับในหน้าเว็บ):
//   68070180/rgb/red    ข้อความ "ON" หรือ "OFF"
//   68070180/rgb/green  ข้อความ "ON" หรือ "OFF"
//   68070180/rgb/blue   ข้อความ "ON" หรือ "OFF"

#include <WiFiS3.h>
#include <MQTTClient.h>

// ===== แก้ตรงนี้ =====
const char WIFI_SSID[] = "iphone 80 promax";             // hotspot มือถือ ต้องเป็น 2.4 GHz
const char WIFI_PASSWORD[] = "88888888";
const char MQTT_BROKER_ADRRESS[] = "mqtt-dashboard.com";
const int   MQTT_PORT = 1883;                    // บอร์ดใช้ TCP 1883 (เว็บใช้ 8884)
const char* TOPIC_SUB = "68070180/rgb/#";   // # = รับทุก topic ย่อย (red, green, blue)

const int R_PIN = 9;
const int B_PIN = 10;
const int G_PIN = 11;
const bool COMMON_ANODE = true;    // RGB ที่แจกเป็น Common Anode: LOW = ติด

WiFiClient network;
MQTTClient mqtt = MQTTClient(256);

void setLed(int pin, bool on) {
  if (COMMON_ANODE) on = !on;
  digitalWrite(pin, on ? HIGH : LOW);
}

void messageReceived(String &topic, String &payload) {
  String t = topic;
  String msg = payload;
  msg.trim();
  msg.toUpperCase();
  bool on = (msg.equalsIgnoreCase("ON") || msg == "1");

  Serial.print("[RX] ");
  Serial.print(t);
  Serial.print(" : ");
  Serial.println(msg);

  if (t.endsWith("/red"))   setLed(R_PIN, on);
  if (t.endsWith("/green")) setLed(G_PIN, on);
  if (t.endsWith("/blue"))  setLed(B_PIN, on);
}

void connectWiFi() {
  Serial.print("Connecting WiFi: ");
  Serial.println(WIFI_SSID);
  while (WiFi.begin(WIFI_SSID, WIFI_PASSWORD) != WL_CONNECTED) {
    Serial.print(".");
    delay(3000);
  }
  while (WiFi.localIP() == IPAddress(0, 0, 0, 0)) delay(200);   // รอจนได้ IP
  Serial.print("WiFi OK, IP = ");
  Serial.println(WiFi.localIP());
}

void connectToMQTT() {
  while (!mqtt.connected()) {
    String clientId = "r4-" + String(random(0xFFFFFF), HEX);   // ห้ามซ้ำกับคนอื่น
    Serial.print("Connecting MQTT ... ");
    if (mqtt.connect(clientId.c_str())) {
      Serial.println("OK");
      mqtt.subscribe(TOPIC_SUB);      // ต้อง subscribe ใหม่ทุกครั้งที่ต่อใหม่
    } else {
      Serial.print("fail err=");
      Serial.print(mqtt.lastError()); Serial.print(" rc="); Serial.println(mqtt.returnCode());
      delay(2000);
    }
  }
}

void setup() {
  Serial.begin(9600);
  pinMode(R_PIN, OUTPUT);
  pinMode(G_PIN, OUTPUT);
  pinMode(B_PIN, OUTPUT);
  setLed(R_PIN, false);
  setLed(G_PIN, false);
  setLed(B_PIN, false);

  connectWiFi();
  randomSeed(micros());
  mqtt.begin(MQTT_BROKER_ADRRESS, MQTT_PORT, network);
  mqtt.onMessage(messageReceived);
  connectToMQTT();
}

void loop() {
  if (WiFi.status() != WL_CONNECTED) connectWiFi();
  if (!mqtt.connected()) connectToMQTT();
  mqtt.loop();        // ต้องเรียกตลอด ห้ามมี delay ยาว ๆ ใน loop
}
