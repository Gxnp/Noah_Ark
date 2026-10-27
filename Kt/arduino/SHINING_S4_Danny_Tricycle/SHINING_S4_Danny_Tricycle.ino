// =====================================================================
// แนวข้อสอบ The Shining  S4 : Danny ปั่นสามล้อเจอฝาแฝด (เดา · แบบ Final 68 ข้อ 4)
// Ultrasonic = ระยะระหว่าง Danny กับฝาแฝดที่ปลายโถง
//   มากกว่า 30 cm   -> publish "Hallway"  LCD: "Danny:" / "Hallway"
//   15 – 30 cm      -> publish "Twins"    LCD: "Danny:" / "Twins"
//   น้อยกว่า 15 cm  -> publish "Play"     LCD: "Come play with" / "us, Danny"
//                                         + Color LED กระพริบฟ้า (สีชุดฝาแฝด)
// publish ไปที่ student_id/danny
//
// บอร์ด Arduino UNO R4 WiFi   ไลบรารี: MQTT (MQTTClient.h), LiquidCrystal_I2C
// วงจร HC-SR04: VCC -> 5V, Trig -> D13, Echo -> D12, GND -> GND
//      Color LED (Common Anode): Common -> 5V, R -> D9, B -> D10, G -> D11 (R 330Ω)
//      LCD I2C: SDA, SCL, 5V, GND
// =====================================================================

#include <WiFiS3.h>
#include <MQTTClient.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

// ===== แก้ตรงนี้ (ข้อความและเกณฑ์ เปลี่ยนให้ตรงโจทย์จริง) =====
const char WIFI_SSID[]  = "iphone 80 promax";
const char WIFI_PASSWORD[]  = "88888888";
const String STUDENT_ID = "68070180";
const char MQTT_BROKER_ADRRESS[] = "phycom.it.kmitl.ac.th";
const int   MQTT_PORT = 1883;
const String TOPIC_NAME = "/danny";
const float NEAR_CM = 15;        // น้อยกว่านี้ = ใกล้มาก
const float FAR_CM  = 30;        // มากกว่านี้ = ไกล

const int TRIG_PIN = 13, ECHO_PIN = 12;
const int R_PIN = 9, B_PIN = 10, G_PIN = 11;

LiquidCrystal_I2C lcd(0x27, 16, 2);
WiFiClient network;
MQTTClient mqtt = MQTTClient(256);
String TOPIC;
String lastState = "";
unsigned long lastRead = 0, lastPub = 0, lastBlink = 0;
bool blinkOn = false;

void lcdLine(int row, String s) {
  lcd.setCursor(0, row);
  while (s.length() < 16) s += ' ';
  lcd.print(s.substring(0, 16));
}

void setColor(int r, int g, int b) {                  // Common Anode
  analogWrite(R_PIN, 255 - r);
  analogWrite(G_PIN, 255 - g);
  analogWrite(B_PIN, 255 - b);
}

float readDistanceCm() {
  digitalWrite(TRIG_PIN, LOW);  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH); delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);
  long us = pulseIn(ECHO_PIN, HIGH, 30000);
  if (us == 0) return -1;
  return us / 29.0 / 2.0;
}

String stateFor(float cm) {
  if (cm < NEAR_CM)  return "Play";
  if (cm <= FAR_CM)  return "Twins";
  return "Hallway";
}

void showState(String s) {
  if (s == "Play") { lcdLine(0, "Come play with"); lcdLine(1, "us, Danny"); }
  else             { lcdLine(0, "Danny:");         lcdLine(1, s); }
  if (s == "Hallway") setColor(0, 0, 0);
  if (s == "Twins")   setColor(0, 120, 255);        // ฟ้า
}

void connectWiFi() {
  lcdLine(0, "WiFi...");
  while (WiFi.begin(WIFI_SSID, WIFI_PASSWORD) != WL_CONNECTED) { Serial.print("."); delay(3000); }
  while (WiFi.localIP() == IPAddress(0, 0, 0, 0)) delay(200);
  Serial.print("WiFi OK, IP = ");
  Serial.println(WiFi.localIP());
}

void connectToMQTT() {
  while (!mqtt.connected()) {
    lcdLine(0, "MQTT...");
    String id = "r4-" + STUDENT_ID + "-" + String(random(0xFFFF), HEX);
    if (mqtt.connect(id.c_str())) { Serial.println("MQTT OK"); lcdLine(0, "Danny:"); }
    else { Serial.print("MQTT fail err="); Serial.print(mqtt.lastError()); Serial.print(" rc="); Serial.println(mqtt.returnCode()); delay(2000); }
  }
}

void setup() {
  Serial.begin(9600);
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
  pinMode(R_PIN, OUTPUT); pinMode(G_PIN, OUTPUT); pinMode(B_PIN, OUTPUT);
  setColor(0, 0, 0);
  lcd.init();
  lcd.backlight();
  TOPIC = STUDENT_ID + TOPIC_NAME;
  connectWiFi();
  randomSeed(micros());
  mqtt.begin(MQTT_BROKER_ADRRESS, MQTT_PORT, network);
  connectToMQTT();
}

void loop() {
  if (WiFi.status() != WL_CONNECTED) connectWiFi();
  if (!mqtt.connected()) connectToMQTT();
  mqtt.loop();

  // ไฟฟ้ากระพริบตอนฝาแฝดอยู่ใกล้ (ไม่ใช้ delay)
  if (lastState == "Play" && millis() - lastBlink >= 300) {
    lastBlink = millis();
    blinkOn = !blinkOn;
    if (blinkOn) setColor(0, 120, 255); else setColor(0, 0, 0);
  }

  if (millis() - lastRead < 300) return;
  lastRead = millis();
  float cm = readDistanceCm();
  if (cm <= 0) return;

  String s = stateFor(cm);
  if (s != lastState || millis() - lastPub >= 2000) {
    mqtt.publish(TOPIC.c_str(), s.c_str());
    Serial.println("[TX] " + String(cm, 1) + " cm -> " + TOPIC + " : " + s);
    lastPub = millis();
  }
  if (s != lastState) {
    lastState = s;
    showState(s);
  }
}
