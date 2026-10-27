// งาน 15.3 : MQTT Analog — ฝั่งคนรับ (Subscribe -> AnalogWrite)
// รับค่า Potentiometer 0–1023 จากบอร์ดเพื่อน แล้วหรี่ไฟ LED ตามค่า
// บอร์ด Arduino UNO R4 WiFi   ไลบรารี: MQTT (MQTTClient.h)
// วงจร: D9 (PWM ~) -> R 220Ω -> ขายาว LED, ขาสั้น -> GND

#include <WiFiS3.h>
#include <MQTTClient.h>

// ===== แก้ตรงนี้ (topic ต้องเหมือนกับฝั่งคนส่ง) =====
const char WIFI_SSID[] = "iphone 80 promax";
const char WIFI_PASSWORD[] = "88888888";
const char MQTT_BROKER_ADRRESS[] = "mqtt-dashboard.com";
const int   MQTT_PORT = 1883;
const char* TOPIC_SUB = "68070180/pot";

const int LED_PIN = 9;     // ขา PWM

WiFiClient network;
MQTTClient mqtt = MQTTClient(256);

void messageReceived(String &topic, String &payload) {
  String msg = payload;
  msg.trim();
  (void)topic;

  int raw = constrain(msg.toInt(), 0, 1023);       // ค่าจาก pot เพื่อน
  int brightness = map(raw, 0, 1023, 0, 255);      // แปลงเป็น PWM 0–255
  analogWrite(LED_PIN, brightness);

  Serial.print("[RX] ");
  Serial.print(raw);
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
  while (WiFi.localIP() == IPAddress(0, 0, 0, 0)) delay(200);
  Serial.print("WiFi OK, IP = ");
  Serial.println(WiFi.localIP());
}

void connectToMQTT() {
  while (!mqtt.connected()) {
    String clientId = "r4-led-" + String(random(0xFFFFFF), HEX);
    Serial.print("Connecting MQTT ... ");
    if (mqtt.connect(clientId.c_str())) {
      Serial.println("OK");
      mqtt.subscribe(TOPIC_SUB);
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
  mqtt.loop();
}
