// งาน 15.1 (แบบ B) : MQTT Dashboard
// บอร์ด publish ค่า Potentiometer ขึ้น HiveMQ -> หน้าเว็บ (dashboard.html) แสดงตัวเลขและแถบ
// หน้าเว็บกดปุ่ม ON/OFF หรือเลื่อนสไลเดอร์ -> publish -> บอร์ด subscribe มาคุมความสว่าง LED
// บอร์ด Arduino UNO R4 WiFi   ไลบรารี: MQTT (Joel Gaehwiler) → MQTTClient.h  |  WiFiS3 มากับบอร์ดแล้ว
// วงจร: Potentiometer ขาริม -> 5V และ GND, ขากลาง -> A0
//       LED: D9 (PWM ~) -> R 220Ω -> ขายาว, ขาสั้น -> GND

#include <WiFiS3.h>
#include <MQTTClient.h>

// ===== แก้ตรงนี้ (topic ต้องตรงกับใน dashboard.html) =====
const char WIFI_SSID[] = "iphone 80 promax";            // hotspot มือถือ ต้องเป็น 2.4 GHz
const char WIFI_PASSWORD[] = "88888888";
const char MQTT_BROKER_ADRRESS[] = "mqtt-dashboard.com";
const int   MQTT_PORT = 1883;                   // บอร์ดใช้ 1883 ส่วนเว็บใช้ 8884
const char* TOPIC_POT = "68070180/pot";    // บอร์ด publish
const char* TOPIC_LED = "68070180/led";    // บอร์ด subscribe

const int POT_PIN = A0;
const int LED_PIN = 9;     // ขา PWM

WiFiClient network;
MQTTClient mqtt = MQTTClient(256);
unsigned long lastPub = 0;

void messageReceived(String &topic, String &payload) {
  String msg = payload;
  msg.trim();

  int brightness;
  if (msg.equalsIgnoreCase("ON"))       brightness = 255;
  else if (msg.equalsIgnoreCase("OFF")) brightness = 0;
  else                   brightness = constrain(msg.toInt(), 0, 255);   // ตัวเลข 0–255 จากสไลเดอร์
  analogWrite(LED_PIN, brightness);

  Serial.print("[RX] ");
  Serial.print(topic);
  Serial.print(" : ");
  Serial.print(msg);
  Serial.print("  ->  LED = ");
  Serial.println(brightness);
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
    String clientId = "r4-dash-" + String(random(0xFFFFFF), HEX);   // ห้ามซ้ำกับคนอื่น
    Serial.print("Connecting MQTT ... ");
    if (mqtt.connect(clientId.c_str())) {
      Serial.println("OK");
      mqtt.subscribe(TOPIC_LED);       // subscribe ใหม่ทุกครั้งที่ต่อใหม่
    } else {
      Serial.print("fail err=");
      Serial.print(mqtt.lastError()); Serial.print(" rc="); Serial.println(mqtt.returnCode());
      delay(2000);
    }
  }
}

void setup() {
  Serial.begin(9600);
  pinMode(LED_PIN, OUTPUT);
  connectWiFi();
  randomSeed(micros());
  mqtt.begin(MQTT_BROKER_ADRRESS, MQTT_PORT, network);
  mqtt.onMessage(messageReceived);
  connectToMQTT();
}

void loop() {
  if (WiFi.status() != WL_CONNECTED) connectWiFi();
  if (!mqtt.connected()) connectToMQTT();
  mqtt.loop();                              // ต้องเรียกตลอด ห้ามใส่ delay ยาว ๆ

  if (millis() - lastPub >= 500) {          // publish ทุก 0.5 วินาที
    lastPub = millis();
    int raw = analogRead(POT_PIN);          // 0–1023
    mqtt.publish(TOPIC_POT, String(raw).c_str());
    Serial.print("[TX] ");
    Serial.print(TOPIC_POT);
    Serial.print(" : ");
    Serial.println(raw);
  }
}
