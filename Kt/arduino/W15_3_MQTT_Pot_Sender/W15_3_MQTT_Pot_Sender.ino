// งาน 15.3 : MQTT Analog — ฝั่งคนส่ง (AnalogRead -> Publish)
// อ่าน Potentiometer แล้ว publish ค่า 0–1023 ขึ้น HiveMQ ให้บอร์ดเพื่อน (W15_3_MQTT_LED_Receiver) หรี่ไฟ
// บอร์ด Arduino UNO R4 WiFi   ไลบรารี: MQTT (MQTTClient.h)
// วงจร: Potentiometer ขาริม -> 5V และ GND, ขากลาง -> A0

#include <WiFiS3.h>
#include <MQTTClient.h>

// ===== แก้ตรงนี้ (topic ต้องเหมือนกับฝั่งคนรับ) =====
const char WIFI_SSID[] = "iphone 80 promax";
const char WIFI_PASSWORD[] = "88888888";
const char MQTT_BROKER_ADRRESS[] = "mqtt-dashboard.com";
const int   MQTT_PORT = 1883;
const char* TOPIC_PUB = "68070180/pot";

const int POT_PIN = A0;

WiFiClient network;
MQTTClient mqtt = MQTTClient(256);
int lastSent = -100;
unsigned long lastPub = 0;

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
    String clientId = "r4-pot-" + String(random(0xFFFFFF), HEX);
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
  connectWiFi();
  randomSeed(micros());
  mqtt.begin(MQTT_BROKER_ADRRESS, MQTT_PORT, network);
  connectToMQTT();
}

void loop() {
  if (WiFi.status() != WL_CONNECTED) connectWiFi();
  if (!mqtt.connected()) connectToMQTT();
  mqtt.loop();

  int raw = analogRead(POT_PIN);   // 0–1023

  // ส่งเมื่อค่าเปลี่ยนเกิน 10 (กันค่าแกว่ง) หรือทุก 2 วินาที และไม่ถี่กว่า 100 ms
  bool changed = abs(raw - lastSent) > 10;
  bool timeout = millis() - lastPub > 2000;
  if ((changed || timeout) && millis() - lastPub > 100) {
    mqtt.publish(TOPIC_PUB, String(raw).c_str());
    Serial.print("[TX] ");
    Serial.print(TOPIC_PUB);
    Serial.print(" : ");
    Serial.println(raw);
    lastSent = raw;
    lastPub = millis();
  }
}
