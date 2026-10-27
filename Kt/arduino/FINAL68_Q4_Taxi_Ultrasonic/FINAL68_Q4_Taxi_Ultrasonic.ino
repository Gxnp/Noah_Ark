// =====================================================================
// Final 68 ข้อ 4 : เครื่องวัดความสบายแท็กซี่ (3 คะแนน)
// อ่าน Ultrasonic จำลองการวัด Leg Room แล้ว
//   น้อยกว่า 15 cm   -> publish "Yaris"  ไปที่ student_id/taxi_model
//   15 – 30 cm       -> publish "Altis"
//   มากกว่า 30 cm    -> publish "Accord"
// และแสดงบน LCD   บรรทัด 1: "Taxi Model:"   บรรทัด 2: ชื่อรุ่น
//
// บอร์ด Arduino UNO R4 WiFi   ไลบรารี: MQTT (MQTTClient.h), LiquidCrystal_I2C
// วงจร HC-SR04: VCC -> 5V, Trig -> D13, Echo -> D12, GND -> GND
// วงจร LCD I2C: GND -> GND, VCC -> 5V, SDA -> SDA, SCL -> SCL
// =====================================================================

#include <WiFiS3.h>
#include <MQTTClient.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

// ===== แก้ตรงนี้ =====
const char WIFI_SSID[]  = "iphone 80 promax";
const char WIFI_PASSWORD[]  = "88888888";
const String STUDENT_ID = "68070180";           // <<< รหัสนักศึกษา

const char MQTT_BROKER_ADRRESS[] = "phycom.it.kmitl.ac.th";
// const char MQTT_BROKER_ADRRESS[] = "mqtt-dashboard.com";
// const char MQTT_BROKER_ADRRESS[] = "test.mosquitto.org";
const int   MQTT_PORT = 1883;

const int TRIG_PIN = 13;
const int ECHO_PIN = 12;

LiquidCrystal_I2C lcd(0x27, 16, 2);              // ถ้าจอไม่ขึ้น ลอง 0x3F
String TOPIC_TAXI;
WiFiClient network;
MQTTClient mqtt = MQTTClient(256);

String lastModel = "";
unsigned long lastRead = 0;
unsigned long lastPub = 0;

void lcdLine(int row, String s) {
  lcd.setCursor(0, row);
  while (s.length() < 16) s += ' ';
  lcd.print(s.substring(0, 16));
}

float readDistanceCm() {
  digitalWrite(TRIG_PIN, LOW);  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH); delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);
  long us = pulseIn(ECHO_PIN, HIGH, 30000);      // รอสูงสุด 30 ms
  if (us == 0) return -1;                        // ไม่มีเสียงสะท้อน
  return us / 29.0 / 2.0;
}

String modelFor(float cm) {
  if (cm < 15)  return "Yaris";                  // น้อยกว่า 15 cm
  if (cm <= 30) return "Altis";                  // 15 – 30 cm
  return "Accord";                               // มากกว่า 30 cm
}

void connectWiFi() {
  lcdLine(0, "WiFi...");
  Serial.print("Connecting WiFi: ");
  Serial.println(WIFI_SSID);
  while (WiFi.begin(WIFI_SSID, WIFI_PASSWORD) != WL_CONNECTED) { Serial.print("."); delay(3000); }
  while (WiFi.localIP() == IPAddress(0, 0, 0, 0)) delay(200);
  Serial.print("WiFi OK, IP = ");
  Serial.println(WiFi.localIP());
}

void connectToMQTT() {
  while (!mqtt.connected()) {
    lcdLine(0, "MQTT...");
    String clientId = "r4-" + STUDENT_ID + "-" + String(random(0xFFFF), HEX);
    if (mqtt.connect(clientId.c_str())) {
      Serial.println("MQTT OK");
      lcdLine(0, "Taxi Model:");
    } else {
      Serial.print("MQTT fail err=");
      Serial.print(mqtt.lastError()); Serial.print(" rc="); Serial.println(mqtt.returnCode());
      delay(2000);
    }
  }
}

void setup() {
  Serial.begin(9600);
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
  lcd.init();
  lcd.backlight();
  TOPIC_TAXI = STUDENT_ID + "/taxi_model";
  connectWiFi();
  randomSeed(micros());
  mqtt.begin(MQTT_BROKER_ADRRESS, MQTT_PORT, network);
  connectToMQTT();
}

void loop() {
  if (WiFi.status() != WL_CONNECTED) connectWiFi();
  if (!mqtt.connected()) connectToMQTT();
  mqtt.loop();

  if (millis() - lastRead < 300) return;         // อ่านทุก 0.3 วินาที
  lastRead = millis();

  float cm = readDistanceCm();
  if (cm <= 0) return;                           // อ่านไม่ได้ ข้ามรอบนี้ ใช้ค่าเดิม

  String model = modelFor(cm);
  Serial.print("Distance = ");
  Serial.print(cm, 1);
  Serial.print(" cm -> ");
  Serial.println(model);

  // ส่งทันทีที่รุ่นเปลี่ยน และส่งซ้ำทุก 2 วินาทีให้เว็บตรวจเห็นค่าล่าสุดเสมอ
  if (model != lastModel || millis() - lastPub >= 2000) {
    mqtt.publish(TOPIC_TAXI.c_str(), model.c_str());
    Serial.println("[TX] " + TOPIC_TAXI + " : " + model);
    lastPub = millis();
  }
  if (model != lastModel) {
    lcdLine(0, "Taxi Model:");
    lcdLine(1, model);
    lastModel = model;
  }
}
