// =====================================================================
// สไลด์ PC69-15 "การทดลองอื่นๆ" : Switch 3 ตัว -> MQTT -> RGB LED
//   คนส่ง (Publisher): กด Switch 1/2/3 -> publish "RED" / "GREEN" / "BLUE"
//                      ปล่อยทุกตัว     -> publish "OFF"
//   คนรับ (Subscriber): ได้ "RED" -> แดง, "GREEN" -> เขียว, "BLUE" -> น้ำเงิน, "OFF" -> ดับ
// ไฟล์นี้ทำได้ทั้ง 2 บทบาทในบอร์ดเดียว (ทำคนเดียว ตั้ง PUBLISH_TOPIC = SUBSCRIBE_TOPIC)
// ถ้าทำ 2 บอร์ด: บอร์ดส่งต่อแค่ Switch, บอร์ดรับต่อแค่ RGB แล้วตั้ง topic ให้ตรงกัน
//
// บอร์ด Arduino UNO R4 WiFi   ไลบรารี: MQTT (Joel Gaehwiler) → MQTTClient.h
// วงจร Switch: D2 / D3 / D4 -> switch -> GND (INPUT_PULLUP)
//      Color LED (Common Anode): Common -> 5V, R -> D9, B -> D10, G -> D11 (R 220–330Ω)
// =====================================================================

#include <WiFiS3.h>
#include <MQTTClient.h>

const char WIFI_SSID[] = "iphone 80 promax";           // CHANGE TO YOUR WIFI SSID
const char WIFI_PASSWORD[] = "88888888";               // CHANGE TO YOUR WIFI PASSWORD

const char MQTT_BROKER_ADRRESS[] = "mqtt-dashboard.com";  // หรือ "phycom.it.kmitl.ac.th"
const int MQTT_PORT = 1883;
const char MQTT_USERNAME[] = "";
const char MQTT_PASSWORD[] = "";

const char PUBLISH_TOPIC[]   = "68070180/switch";      // คนส่ง
const char SUBSCRIBE_TOPIC[] = "68070180/switch";      // คนรับ (2 บอร์ดให้ตรงกับของคนส่ง)

const int SW_PIN[3] = {2, 3, 4};
const char* SW_COLOR[3] = {"RED", "GREEN", "BLUE"};
const int R_PIN = 9, B_PIN = 10, G_PIN = 11;

WiFiClient network;
MQTTClient mqtt = MQTTClient(256);

int lastPressed = -2;          // switch ที่กดอยู่ล่าสุด (-1 = ไม่ได้กด)

void setRgb(bool r, bool g, bool b) {                  // Common Anode: LOW = ติด
  digitalWrite(R_PIN, r ? LOW : HIGH);
  digitalWrite(G_PIN, g ? LOW : HIGH);
  digitalWrite(B_PIN, b ? LOW : HIGH);
}

void messageReceived(String &topic, String &payload) {
  Serial.println("Arduino UNO R4 - received from MQTT:");
  Serial.println("- topic: " + topic);
  Serial.println("- payload: " + payload);

  String msg = payload;
  msg.trim();
  msg.toUpperCase();
  if (msg == "RED")        setRgb(1, 0, 0);
  else if (msg == "GREEN") setRgb(0, 1, 0);
  else if (msg == "BLUE")  setRgb(0, 0, 1);
  else if (msg == "OFF")   setRgb(0, 0, 0);
  // รับเป็นเลขได้ด้วย: "1" "2" "3" "0"
  else if (msg == "1")     setRgb(1, 0, 0);
  else if (msg == "2")     setRgb(0, 1, 0);
  else if (msg == "3")     setRgb(0, 0, 1);
  else if (msg == "0")     setRgb(0, 0, 0);
}

void connectToMQTT() {
  mqtt.begin(MQTT_BROKER_ADRRESS, MQTT_PORT, network);
  mqtt.onMessage(messageReceived);

  Serial.print("Arduino UNO R4 - Connecting to MQTT broker");
  String clientId = "68070180-sw-" + String(random(0xFFFF), HEX);    // ห้ามซ้ำ
  while (!mqtt.connect(clientId.c_str(), MQTT_USERNAME, MQTT_PASSWORD)) {
    Serial.print(".");
    delay(100);
  }
  Serial.println();
  mqtt.subscribe(SUBSCRIBE_TOPIC);
  Serial.print("Arduino UNO R4 - Subscribed to the topic: ");
  Serial.println(SUBSCRIBE_TOPIC);
}

void setup() {
  Serial.begin(9600);
  for (int i = 0; i < 3; i++) pinMode(SW_PIN[i], INPUT_PULLUP);
  pinMode(R_PIN, OUTPUT); pinMode(G_PIN, OUTPUT); pinMode(B_PIN, OUTPUT);
  setRgb(0, 0, 0);

  int status = WL_IDLE_STATUS;
  while (status != WL_CONNECTED) {
    Serial.print("Arduino UNO R4 - Attempting to connect to SSID: ");
    Serial.println(WIFI_SSID);
    status = WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    delay(5000);
  }
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());
  randomSeed(micros());
  connectToMQTT();
}

void loop() {
  mqtt.loop();
  if (!mqtt.connected()) connectToMQTT();

  // หา switch ที่กดอยู่ (ถ้ากดหลายตัว เอาตัวแรก)
  int pressed = -1;
  for (int i = 0; i < 3; i++) {
    if (digitalRead(SW_PIN[i]) == LOW) { pressed = i; break; }
  }

  // publish เฉพาะตอนสถานะเปลี่ยน (กดใหม่ / ปล่อย)
  if (pressed != lastPressed) {
    delay(30);                                     // กันสัญญาณเด้ง
    const char* msg = (pressed >= 0) ? SW_COLOR[pressed] : "OFF";
    mqtt.publish(PUBLISH_TOPIC, msg);
    Serial.print("Arduino UNO R4 - sent to MQTT: ");
    Serial.println(msg);
    lastPressed = pressed;
  }
}
