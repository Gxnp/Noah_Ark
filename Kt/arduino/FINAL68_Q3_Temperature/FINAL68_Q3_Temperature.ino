// =====================================================================
// Final 68 ข้อ 3 : อุณหภูมิบนดาดฟ้า Marina Bay Sands (1 คะแนน)
// อ่านค่าเซ็นเซอร์วัดอุณหภูมิ แล้ว publish ไปที่ student_id/temp
//
// บอร์ด Arduino UNO R4 WiFi   ไลบรารี: MQTT (MQTTClient.h)
// วงจร MCP9700 (หันด้านแบนที่มีตัวหนังสือเข้าหาตัว):
//   ขาซ้าย VDD -> 5V, ขากลาง Vout -> A1, ขาขวา GND -> GND
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

const int TEMP_PIN = A1;
// ชนิดเซ็นเซอร์: 1 = MCP9700 / TMP36 (0.5 V ที่ 0 °C)   2 = LM35 (0 V ที่ 0 °C)
const int SENSOR_TYPE = 1;

String TOPIC_TEMP;
WiFiClient network;
MQTTClient mqtt = MQTTClient(256);
unsigned long lastPub = 0;

float readTempC() {
  // เฉลี่ย 10 ครั้ง ค่าจะนิ่งขึ้น
  long sum = 0;
  for (int i = 0; i < 10; i++) { sum += analogRead(TEMP_PIN); delay(2); }
  float v = (sum / 10.0) * 5.0 / 1023.0;
  if (SENSOR_TYPE == 2) return v * 100.0;        // LM35: 10 mV/°C
  return (v - 0.5) / 0.01;                       // MCP9700: 0.5 V = 0 °C, 10 mV/°C
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
    } else {
      Serial.print("fail err=");
      Serial.print(mqtt.lastError()); Serial.print(" rc="); Serial.println(mqtt.returnCode());
      delay(2000);
    }
  }
}

void setup() {
  Serial.begin(9600);
  TOPIC_TEMP = STUDENT_ID + "/temp";
  connectWiFi();
  randomSeed(micros());
  mqtt.begin(MQTT_BROKER_ADRRESS, MQTT_PORT, network);
  connectToMQTT();
}

void loop() {
  if (WiFi.status() != WL_CONNECTED) connectWiFi();
  if (!mqtt.connected()) connectToMQTT();
  mqtt.loop();

  if (millis() - lastPub >= 1000) {              // ส่งทุก 1 วินาที
    lastPub = millis();
    float t = readTempC();
    String payload = String(t, 1);               // เช่น "27.4"
    mqtt.publish(TOPIC_TEMP.c_str(), payload.c_str());
    Serial.print("[TX] ");
    Serial.print(TOPIC_TEMP);
    Serial.print(" : ");
    Serial.println(payload);
  }
}
