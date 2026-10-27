// =====================================================================
// Final 68 รวมทุกข้อในไฟล์เดียว (ต่อวงจรครั้งเดียว ทำได้ทั้ง 5 ข้อพร้อมกัน)
//   ข้อ 1  Potentiometer (A0)        -> publish  student_id/emailspin
//   ข้อ 2  subscribe student_id/prob_stat -> 7-Segment 1 หลัก (D2–D8)
//   ข้อ 3  MCP9700 (A1)              -> publish  student_id/temp
//   ข้อ 4  Ultrasonic (D13/D12)      -> publish  student_id/taxi_model + LCD
//   ข้อ 5  subscribe queue x3        -> publish  student_id/sentosa_choice + LCD + Color LED
//   ปุ่มกด (A2) สลับหน้าจอ LCD ระหว่างข้อ 4 (Taxi) กับข้อ 5 (Sentosa)
//
// บอร์ด Arduino UNO R4 WiFi   ไลบรารี: MQTT (MQTTClient.h), LiquidCrystal_I2C
// ---------------------------------------------------------------------
// ตารางขา (ตัวต้านทาน 330Ω ทุกเส้นของ 7-Segment และ Color LED)
//   D2–D8  : 7-Segment a b c d e f g        Common -> GND (Common Cathode)
//   D9     : Color LED R                    Common -> 5V  (Common Anode)
//   D10    : Color LED B
//   D11    : Color LED G
//   D12    : Ultrasonic Echo
//   D13    : Ultrasonic Trig                VCC -> 5V, GND -> GND
//   A0     : Potentiometer ขากลาง           ขาริม -> 5V และ GND
//   A1     : MCP9700 Vout                   VDD -> 5V, GND -> GND
//   A2     : Push Button -> GND             (INPUT_PULLUP ไม่ต้องมีตัวต้านทาน)
//   SDA/SCL: LCD I2C                        VCC -> 5V, GND -> GND
// =====================================================================

#include <WiFiS3.h>
#include <MQTTClient.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

// ===== แก้ตรงนี้ =====
const char WIFI_SSID[]  = "iphone 80 promax";            // hotspot มือถือ 2.4 GHz
const char WIFI_PASSWORD[]  = "88888888";
const String STUDENT_ID = "68070180";            // <<< รหัสนักศึกษา

const char MQTT_BROKER_ADRRESS[] = "phycom.it.kmitl.ac.th";
// const char MQTT_BROKER_ADRRESS[] = "mqtt-dashboard.com";
// const char MQTT_BROKER_ADRRESS[] = "test.mosquitto.org";
const int   MQTT_PORT = 1883;

const bool SPIN_PERCENT = false;   // ข้อ 1: false = ส่ง 0–1023, true = ส่ง 0–100
const int  TEMP_SENSOR  = 1;       // ข้อ 3: 1 = MCP9700/TMP36, 2 = LM35

// ----- ขา -----
const int SEG_PINS[7] = {2, 3, 4, 5, 6, 7, 8};    // a b c d e f g
const int R_PIN = 9, B_PIN = 10, G_PIN = 11;
const int ECHO_PIN = 12, TRIG_PIN = 13;
const int POT_PIN = A0, TEMP_PIN = A1, BUTTON_PIN = A2;
const bool SEG_COMMON_ANODE = false;
const bool RGB_COMMON_ANODE = true;

LiquidCrystal_I2C lcd(0x27, 16, 2);               // ถ้าจอไม่ขึ้น ลอง 0x3F
WiFiClient network;
MQTTClient mqtt = MQTTClient(256);

// ----- topic -----
String T_SPIN, T_GATE, T_TEMP, T_TAXI, T_CHOICE;
const char* PLACE_NAME[3]   = {"Universal", "Aquarium", "Luge"};
const char* QUEUE_SUFFIX[3] = {"/queue_chotiphat", "/queue_lapas", "/queue_sumet"};

// ----- สถานะ -----
enum Page { PAGE_TAXI, PAGE_SENTOSA };
Page page = PAGE_TAXI;
String taxiModel = "-";
String sentosaLine1 = "Waiting queue..";
String sentosaLine2 = "";
float queueMin[3] = {-1, -1, -1};
bool queueUpdated = false;

int lastSpin = -100;
unsigned long tSpin = 0, tTemp = 0, tTaxiRead = 0, tTaxiPub = 0, tQueue = 0;
bool lastButton = HIGH;
unsigned long tButton = 0;

// =====================================================================
// แสดงผล
// =====================================================================
void lcdLine(int row, String s) {
  lcd.setCursor(0, row);
  while (s.length() < 16) s += ' ';
  lcd.print(s.substring(0, 16));
}

void drawPage() {
  if (page == PAGE_TAXI) {
    lcdLine(0, "Taxi Model:");
    lcdLine(1, taxiModel);
  } else {
    lcdLine(0, sentosaLine1);
    lcdLine(1, sentosaLine2);
  }
}

const byte DIGITS[10][7] = {
  {1,1,1,1,1,1,0}, {0,1,1,0,0,0,0}, {1,1,0,1,1,0,1}, {1,1,1,1,0,0,1}, {0,1,1,0,0,1,1},
  {1,0,1,1,0,1,1}, {1,0,1,1,1,1,1}, {1,1,1,0,0,0,0}, {1,1,1,1,1,1,1}, {1,1,1,1,0,1,1}
};

void writeSeg(int i, bool on) {
  if (SEG_COMMON_ANODE) on = !on;
  digitalWrite(SEG_PINS[i], on ? HIGH : LOW);
}
void showDigit(int n) { for (int i = 0; i < 7; i++) writeSeg(i, DIGITS[n][i]); }
void showDash()       { for (int i = 0; i < 7; i++) writeSeg(i, i == 6); }

void setColor(int r, int g, int b) {
  if (RGB_COMMON_ANODE) { r = 255 - r; g = 255 - g; b = 255 - b; }
  analogWrite(R_PIN, r);
  analogWrite(G_PIN, g);
  analogWrite(B_PIN, b);
}

// =====================================================================
// อ่านค่าเซ็นเซอร์
// =====================================================================
float readTempC() {
  long sum = 0;
  for (int i = 0; i < 10; i++) { sum += analogRead(TEMP_PIN); delay(2); }
  float v = (sum / 10.0) * 5.0 / 1023.0;
  if (TEMP_SENSOR == 2) return v * 100.0;
  return (v - 0.5) / 0.01;
}

float readDistanceCm() {
  digitalWrite(TRIG_PIN, LOW);  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH); delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);
  long us = pulseIn(ECHO_PIN, HIGH, 30000);
  if (us == 0) return -1;
  return us / 29.0 / 2.0;
}

String modelFor(float cm) {
  if (cm < 15)  return "Yaris";
  if (cm <= 30) return "Altis";
  return "Accord";
}

// =====================================================================
// แต่ละข้อ
// =====================================================================
void q1_emailspin() {
  int raw = analogRead(POT_PIN);
  int value = SPIN_PERCENT ? map(raw, 0, 1023, 0, 100) : raw;
  int threshold = SPIN_PERCENT ? 1 : 8;
  bool changed = abs(value - lastSpin) >= threshold;
  if ((changed || millis() - tSpin >= 1000) && millis() - tSpin >= 100) {
    mqtt.publish(T_SPIN.c_str(), String(value).c_str());
    Serial.println("[Q1] " + T_SPIN + " : " + String(value));
    lastSpin = value;
    tSpin = millis();
  }
}

void q3_temp() {
  if (millis() - tTemp < 1000) return;
  tTemp = millis();
  String payload = String(readTempC(), 1);
  mqtt.publish(T_TEMP.c_str(), payload.c_str());
  Serial.println("[Q3] " + T_TEMP + " : " + payload);
}

void q4_taxi() {
  if (millis() - tTaxiRead < 300) return;
  tTaxiRead = millis();
  float cm = readDistanceCm();
  if (cm <= 0) return;
  String model = modelFor(cm);
  if (model != taxiModel || millis() - tTaxiPub >= 2000) {
    mqtt.publish(T_TAXI.c_str(), model.c_str());
    Serial.println("[Q4] " + String(cm, 1) + " cm -> " + T_TAXI + " : " + model);
    tTaxiPub = millis();
  }
  if (model != taxiModel) {
    taxiModel = model;
    if (page == PAGE_TAXI) drawPage();
  }
}

void q5_sentosa() {
  if (!queueUpdated || millis() - tQueue < 500) return;
  queueUpdated = false;
  tQueue = millis();

  for (int i = 0; i < 3; i++) {
    if (queueMin[i] < 0) {
      sentosaLine1 = "Waiting queue..";
      sentosaLine2 = String(PLACE_NAME[i]) + " ?";
      if (page == PAGE_SENTOSA) drawPage();
      return;
    }
  }

  int best = 0;
  for (int i = 1; i < 3; i++) if (queueMin[i] < queueMin[best]) best = i;
  float avg = (queueMin[0] + queueMin[1] + queueMin[2]) / 3.0;

  mqtt.publish(T_CHOICE.c_str(), PLACE_NAME[best]);

  sentosaLine1 = String(PLACE_NAME[best]) + " (" + String((int)round(queueMin[best])) + "m)";
  sentosaLine2 = "Avg: " + String((int)round(avg)) + "m";
  if (page == PAGE_SENTOSA) drawPage();

  if (avg > 60)       setColor(255, 0, 0);       // แดง
  else if (avg >= 30) setColor(255, 70, 0);      // ส้ม
  else                setColor(0, 255, 120);     // เขียวอมฟ้า

  Serial.println("[Q5] " + T_CHOICE + " : " + PLACE_NAME[best] + "  avg " + String(avg, 1));
}

void checkButton() {
  bool now = digitalRead(BUTTON_PIN);
  if (now != lastButton && millis() - tButton > 50) {   // กันสัญญาณเด้ง 50 ms
    tButton = millis();
    lastButton = now;
    if (now == LOW) {                                   // กดลง
      page = (page == PAGE_TAXI) ? PAGE_SENTOSA : PAGE_TAXI;
      drawPage();
      Serial.println(page == PAGE_TAXI ? "LCD -> Taxi" : "LCD -> Sentosa");
    }
  }
}

// =====================================================================
// MQTT รับข้อความ (ข้อ 2 และข้อ 5)
// =====================================================================
void messageReceived(String &topic, String &payload) {
  String t = topic;
  String msg = payload;
  msg.trim();

  if (t == T_GATE) {                                    // ข้อ 2
    Serial.println("[Q2] gate = " + msg);
    for (unsigned int i = 0; i < msg.length(); i++) {
      if (isDigit(msg[i])) { showDigit(msg[i] - '0'); return; }
    }
    showDash();
    return;
  }

  for (int i = 0; i < 3; i++) {                         // ข้อ 5
    if (t.endsWith(QUEUE_SUFFIX[i])) {
      queueMin[i] = msg.toFloat();
      queueUpdated = true;
    }
  }
}

// =====================================================================
// WiFi + MQTT
// =====================================================================
void connectWiFi() {
  lcdLine(0, "WiFi...");
  lcdLine(1, WIFI_SSID);
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
    lcdLine(1, MQTT_BROKER_ADRRESS);
    String clientId = "r4-" + STUDENT_ID + "-" + String(random(0xFFFF), HEX);
    if (mqtt.connect(clientId.c_str())) {
      mqtt.subscribe(T_GATE.c_str());
      for (int i = 0; i < 3; i++) mqtt.subscribe((STUDENT_ID + QUEUE_SUFFIX[i]).c_str());
      Serial.println("MQTT OK");
      drawPage();
    } else {
      Serial.print("MQTT fail err=");
      Serial.print(mqtt.lastError()); Serial.print(" rc="); Serial.println(mqtt.returnCode());
      delay(2000);
    }
  }
}

void setup() {
  Serial.begin(9600);
  for (int i = 0; i < 7; i++) pinMode(SEG_PINS[i], OUTPUT);
  pinMode(R_PIN, OUTPUT); pinMode(G_PIN, OUTPUT); pinMode(B_PIN, OUTPUT);
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  showDash();
  setColor(0, 0, 0);
  lcd.init();
  lcd.backlight();

  T_SPIN   = STUDENT_ID + "/emailspin";
  T_GATE   = STUDENT_ID + "/prob_stat";
  T_TEMP   = STUDENT_ID + "/temp";
  T_TAXI   = STUDENT_ID + "/taxi_model";
  T_CHOICE = STUDENT_ID + "/sentosa_choice";

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

  checkButton();
  q1_emailspin();
  q3_temp();
  q4_taxi();
  q5_sentosa();
}
