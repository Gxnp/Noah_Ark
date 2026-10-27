// =====================================================================
// เทมเพลตวันสอบ: อ่านค่าจากอุปกรณ์ -> publish ขึ้น broker
//                subscribe ค่ากลับมา -> แสดงผลที่อุปกรณ์ (LCD / LED / RGB)
// บอร์ด Arduino UNO R4 WiFi
// ไลบรารี: MQTT (MQTTClient.h), LiquidCrystal_I2C   (WiFiS3 มากับบอร์ดแล้ว)
//
// ขาที่ใช้ (ต่อเฉพาะตัวที่โจทย์ให้ใช้)
//   Potentiometer : ขากลาง -> A0 (ขาริม 5V, GND)
//   MCP9700       : Vout -> A1
//   Thermistor    : 5V -> R10k -> A2 -> Thermistor -> GND
//   Ultrasonic    : Trig -> D4, Echo -> D2
//   ปุ่มกด         : D7 -> ปุ่ม -> GND (ใช้ INPUT_PULLUP ไม่ต้องมี R)
//   LED           : D3 -> R 220Ω -> LED -> GND
//   RGB (C.Anode) : Common -> 5V, R -> D9, B -> D10, G -> D11 (ผ่าน R 220Ω)
//   LCD I2C       : SDA -> SDA/A4, SCL -> SCL/A5, VCC -> 5V
//
// ต้องแก้แค่ 3 จุด: (1) WiFi/Topic  (2) เลือกเซนเซอร์  (3) ช่วงค่าที่ใช้แสดงผล
// =====================================================================

#include <WiFiS3.h>
#include <MQTTClient.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <math.h>

// ===== (1) WiFi / Broker / Topic =====
const char WIFI_SSID[] = "iphone 80 promax";               // hotspot มือถือ 2.4 GHz
const char WIFI_PASSWORD[] = "88888888";
const char MQTT_BROKER_ADRRESS[] = "mqtt-dashboard.com";       // ใช้ broker ตามที่ข้อสอบกำหนด
const int   MQTT_PORT = 1883;
const char* TOPIC_PUB = "68070180/sensor";    // topic ที่เราส่งค่า
const char* TOPIC_SUB = "68070180/sensor";    // topic ที่เรารับมาแสดง
// ทำคนเดียว: ตั้ง PUB กับ SUB เหมือนกัน = ส่งเองรับเอง
// ทำเป็นคู่ / โจทย์กำหนด topic อื่น: เปลี่ยน TOPIC_SUB ตามนั้น

// ===== (2) เลือกเซนเซอร์ =====
// 1 = Potentiometer (0–1023)   2 = Ultrasonic (cm)   3 = MCP9700 (°C)
// 4 = Thermistor (°C)          5 = ปุ่มกด (1 = กด, 0 = ไม่กด)
const int SENSOR = 1;
const unsigned long PUB_INTERVAL = 1000;           // ส่งทุก 1 วินาที

// ===== (3) ช่วงค่าที่จะแปลงเป็นความสว่าง LED / สี RGB =====
// เช่น Pot 0–1023, Ultrasonic 0–100 cm, อุณหภูมิ 25–40 °C, ปุ่ม 0–1
const float VAL_MIN = 0;
const float VAL_MAX = 1023;
const bool  USE_RGB = false;      // true = แสดงสีตามค่าด้วย (ต่ำ=เขียว, กลาง=เหลือง, สูง=แดง)

// ----- ขา -----
const int POT_PIN = A0, MCP_PIN = A1, THERM_PIN = A2;
const int TRIG_PIN = 4, ECHO_PIN = 2, BUTTON_PIN = 7;
const int LED_PIN = 3;
const int R_PIN = 9, B_PIN = 10, G_PIN = 11;

LiquidCrystal_I2C lcd(0x27, 16, 2);   // ถ้าจอไม่ขึ้น ลอง 0x3F
WiFiClient network;
MQTTClient mqtt = MQTTClient(512);
unsigned long lastPub = 0;

// ---------------------------------------------------------------------
// ฟังก์ชันอ่านเซนเซอร์ (ก๊อปมาจากงานสัปดาห์ 13–14)
// ---------------------------------------------------------------------
float readPot() { return analogRead(POT_PIN); }

float readUltrasonicCm() {
  digitalWrite(TRIG_PIN, LOW);  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH); delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);
  long us = pulseIn(ECHO_PIN, HIGH, 30000);
  if (us == 0) return -1;                 // ไม่มีเสียงสะท้อน
  return us / 29.0 / 2.0;
}

float readMcp9700C() {
  float v = analogRead(MCP_PIN) * 5.0 / 1023.0;
  return (v - 0.5) / 0.01;
}

float readThermistorC() {
  int raw = analogRead(THERM_PIN);
  if (raw <= 0 || raw >= 1023) return NAN;
  float rTh = 10000.0 * raw / (1023.0 - raw);
  float tK = 1.0 / (1.0 / 298.15 + log(rTh / 10000.0) / 4050.0);
  return tK - 273.15;
}

float readButton() { return digitalRead(BUTTON_PIN) == LOW ? 1 : 0; }

String readSensor() {
  switch (SENSOR) {
    case 1: return String((int)readPot());
    case 2: return String(readUltrasonicCm(), 1);
    case 3: return String(readMcp9700C(), 1);
    case 4: return String(readThermistorC(), 1);
    case 5: return String((int)readButton());
  }
  return "0";
}

// ---------------------------------------------------------------------
// แสดงผล
// ---------------------------------------------------------------------
void lcdLine(int row, String s) {
  lcd.setCursor(0, row);
  while (s.length() < 16) s += ' ';
  lcd.print(s.substring(0, 16));
}

void setRgb(bool r, bool g, bool b) {        // Common Anode: LOW = ติด
  digitalWrite(R_PIN, r ? LOW : HIGH);
  digitalWrite(G_PIN, g ? LOW : HIGH);
  digitalWrite(B_PIN, b ? LOW : HIGH);
}

void messageReceived(String &topic, String &payload) {
  String msg = payload;
  msg.trim();
  Serial.print("[RX] ");
  Serial.print(topic);
  Serial.print(" : ");
  Serial.println(msg);

  lcdLine(1, "RX: " + msg);                                  // LCD บรรทัดล่าง

  if (msg.equalsIgnoreCase("ON"))  { analogWrite(LED_PIN, 255); return; }   // คำสั่งข้อความ
  if (msg.equalsIgnoreCase("OFF")) { analogWrite(LED_PIN, 0);   return; }

  float value = msg.toFloat();
  float ratio = constrain((value - VAL_MIN) / (VAL_MAX - VAL_MIN), 0.0, 1.0);   // 0.0–1.0
  analogWrite(LED_PIN, (int)(ratio * 255));                  // LED หรี่ตามค่า

  if (USE_RGB) {
    if (ratio < 0.33)      setRgb(0, 1, 0);   // ต่ำ = เขียว
    else if (ratio < 0.66) setRgb(1, 1, 0);   // กลาง = เหลือง
    else                   setRgb(1, 0, 0);   // สูง = แดง
  }
}

// ---------------------------------------------------------------------
// WiFi + MQTT
// ---------------------------------------------------------------------
void connectWiFi() {
  lcdLine(0, "WiFi...");
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
    lcdLine(0, "MQTT...");
    String clientId = "r4-exam-" + String(random(0xFFFFFF), HEX);
    if (mqtt.connect(clientId.c_str())) {
      mqtt.subscribe(TOPIC_SUB);
      Serial.println("MQTT OK");
      lcdLine(0, "MQTT OK");
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
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  pinMode(LED_PIN, OUTPUT);
  pinMode(R_PIN, OUTPUT); pinMode(G_PIN, OUTPUT); pinMode(B_PIN, OUTPUT);
  setRgb(0, 0, 0);

  lcd.init();
  lcd.backlight();
  connectWiFi();
  randomSeed(micros());
  mqtt.begin(MQTT_BROKER_ADRRESS, MQTT_PORT, network);
  mqtt.onMessage(messageReceived);
  connectToMQTT();
}

void loop() {
  if (WiFi.status() != WL_CONNECTED) connectWiFi();
  if (!mqtt.connected()) connectToMQTT();
  mqtt.loop();                                   // ห้ามใส่ delay ยาว ๆ ใน loop

  if (millis() - lastPub >= PUB_INTERVAL) {
    lastPub = millis();
    String value = readSensor();
    mqtt.publish(TOPIC_PUB, value.c_str());
    Serial.print("[TX] ");
    Serial.print(TOPIC_PUB);
    Serial.print(" : ");
    Serial.println(value);
    lcdLine(0, "TX: " + value);                 // LCD บรรทัดบน = ค่าที่เราส่ง
  }
}
