// =====================================================================
// แนวข้อสอบ The Shining  S6 : หนีจากโรงแรม Overlook (เดา · แบบ Final 68 ข้อ 5)
// subscribe "ระดับผี" 0–100 ของ 3 ที่ ที่ส่งมาทุก 1–2 วินาที
//   student_id/ghost_room237   = Room 237
//   student_id/ghost_goldroom  = Gold Room
//   student_id/ghost_maze      = Hedge Maze
// แล้ว
//   1) publish ชื่อที่ปลอดภัยที่สุด (ค่าน้อยสุด) ไปที่ student_id/escape  ("Room237" / "GoldRoom" / "Maze")
//   2) หาค่าเฉลี่ย
//   3) LCD  บรรทัด 1: "Maze (12)"   บรรทัด 2: "Avg: 45"
//   4) Color LED ตามค่าเฉลี่ย  > 60 = แดง (REDRUM),  30–60 = ส้ม,  < 30 = เขียว
//
// บอร์ด Arduino UNO R4 WiFi   ไลบรารี: MQTT (MQTTClient.h), LiquidCrystal_I2C
// วงจร Color LED (Common Anode): Common -> 5V, R -> D9, B -> D10, G -> D11 (R 330Ω)
//      LCD I2C: SDA, SCL, 5V, GND
// =====================================================================

#include <WiFiS3.h>
#include <MQTTClient.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

// ===== แก้ตรงนี้ (ชื่อ topic / ชื่อที่ / เกณฑ์ เปลี่ยนให้ตรงโจทย์จริง) =====
const char WIFI_SSID[]  = "iphone 80 promax";
const char WIFI_PASSWORD[]  = "88888888";
const String STUDENT_ID = "68070180";
const char MQTT_BROKER_ADRRESS[] = "phycom.it.kmitl.ac.th";
const int   MQTT_PORT = 1883;

const int N = 3;                                                    // จำนวนแหล่งข้อมูล
const char* NAME[N]   = {"Room237", "GoldRoom", "Maze"};
const char* SUFFIX[N] = {"/ghost_room237", "/ghost_goldroom", "/ghost_maze"};
const String TOPIC_OUT = "/escape";
const String UNIT = "";                                             // เช่น "m" ถ้าเป็นนาที
const float HIGH_LEVEL = 60, LOW_LEVEL = 30;

const int R_PIN = 9, B_PIN = 10, G_PIN = 11;

LiquidCrystal_I2C lcd(0x27, 16, 2);
WiFiClient network;
MQTTClient mqtt = MQTTClient(256);
float value[N];
bool updated = false;
unsigned long lastCalc = 0;
String T_OUT;

void lcdLine(int row, String s) {
  lcd.setCursor(0, row);
  while (s.length() < 16) s += ' ';
  lcd.print(s.substring(0, 16));
}

void setColor(int r, int g, int b) {                                // Common Anode
  analogWrite(R_PIN, 255 - r);
  analogWrite(G_PIN, 255 - g);
  analogWrite(B_PIN, 255 - b);
}

void messageReceived(String &topic, String &payload) {
  String t = topic;
  String msg = payload;
  msg.trim();
  for (int i = 0; i < N; i++) {
    if (t.endsWith(SUFFIX[i])) {
      value[i] = msg.toFloat();
      updated = true;
      Serial.println("[RX] " + String(NAME[i]) + " = " + msg);
    }
  }
}

void process() {
  for (int i = 0; i < N; i++) {
    if (value[i] < 0) { lcdLine(0, "Waiting data.."); lcdLine(1, String(NAME[i]) + " ?"); return; }
  }
  int best = 0;
  float sum = 0;
  for (int i = 0; i < N; i++) {
    sum += value[i];
    if (value[i] < value[best]) best = i;
  }
  float avg = sum / N;

  mqtt.publish(T_OUT.c_str(), NAME[best]);
  lcdLine(0, String(NAME[best]) + " (" + String((int)round(value[best])) + UNIT + ")");
  lcdLine(1, "Avg: " + String((int)round(avg)) + UNIT);

  if (avg > HIGH_LEVEL)      setColor(255, 0, 0);     // แดง
  else if (avg >= LOW_LEVEL) setColor(255, 70, 0);    // ส้ม
  else                       setColor(0, 255, 0);     // เขียว

  Serial.println("[TX] " + T_OUT + " : " + NAME[best] + "   avg " + String(avg, 1));
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
    if (mqtt.connect(id.c_str())) {
      for (int i = 0; i < N; i++) mqtt.subscribe((STUDENT_ID + SUFFIX[i]).c_str());
      Serial.println("MQTT OK");
      lcdLine(0, "Waiting data..");
      lcdLine(1, "");
    } else { Serial.print("MQTT fail err="); Serial.print(mqtt.lastError()); Serial.print(" rc="); Serial.println(mqtt.returnCode()); delay(2000); }
  }
}

void setup() {
  Serial.begin(9600);
  for (int i = 0; i < N; i++) value[i] = -1;
  pinMode(R_PIN, OUTPUT); pinMode(G_PIN, OUTPUT); pinMode(B_PIN, OUTPUT);
  setColor(0, 0, 0);
  lcd.init();
  lcd.backlight();
  T_OUT = STUDENT_ID + TOPIC_OUT;
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
  if (updated && millis() - lastCalc >= 500) {
    updated = false;
    lastCalc = millis();
    process();
  }
}
