// =====================================================================
// แนวข้อสอบ The Shining  S5 : "Here's Johnny!" Jack ฟันประตูด้วยขวาน (เดา)
// กด Push Button 1 ครั้ง = ฟัน 1 ที
//   ทุกครั้งที่ฟัน      -> publish จำนวนครั้งไปที่ student_id/axe   LCD: "Axe chops:" / "3 / 5"
//   ฟันครบ CHOPS ครั้ง -> publish "Here's Johnny!" ไปที่ student_id/door
//                         LCD: "Here's Johnny!" / "Door is broken"
//                         Color LED กระพริบแดง 3 วินาที แล้วเริ่มนับใหม่
// รับคำสั่งรีเซ็ตได้: publish "reset" ไปที่ student_id/door_cmd
//
// บอร์ด Arduino UNO R4 WiFi   ไลบรารี: MQTT (MQTTClient.h), LiquidCrystal_I2C
// วงจร Push Button: ขา 1 -> A2, ขาทแยง -> GND (INPUT_PULLUP ไม่ต้องมีตัวต้านทาน)
//      Color LED (Common Anode): Common -> 5V, R -> D9, B -> D10, G -> D11 (R 330Ω)
//      LCD I2C: SDA, SCL, 5V, GND
// =====================================================================

#include <WiFiS3.h>
#include <MQTTClient.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

// ===== แก้ตรงนี้ =====
const char WIFI_SSID[]  = "iphone 80 promax";
const char WIFI_PASSWORD[]  = "88888888";
const String STUDENT_ID = "68070180";
const char MQTT_BROKER_ADRRESS[] = "phycom.it.kmitl.ac.th";
const int   MQTT_PORT = 1883;
const String TOPIC_AXE  = "/axe";
const String TOPIC_DOOR = "/door";
const String TOPIC_CMD  = "/door_cmd";
const int CHOPS = 5;                                   // ฟันกี่ทีประตูพัง

const int BUTTON_PIN = A2;
const int R_PIN = 9, B_PIN = 10, G_PIN = 11;

LiquidCrystal_I2C lcd(0x27, 16, 2);
WiFiClient network;
MQTTClient mqtt = MQTTClient(256);
String T_AXE, T_DOOR, T_CMD;

int chops = 0;
bool lastButton = HIGH;
unsigned long tButton = 0;
unsigned long brokenAt = 0;                            // 0 = ประตูยังไม่พัง

void lcdLine(int row, String s) {
  lcd.setCursor(0, row);
  while (s.length() < 16) s += ' ';
  lcd.print(s.substring(0, 16));
}

void setColor(int r, int g, int b) {                   // Common Anode
  analogWrite(R_PIN, 255 - r);
  analogWrite(G_PIN, 255 - g);
  analogWrite(B_PIN, 255 - b);
}

void showCount() {
  lcdLine(0, "Axe chops:");
  lcdLine(1, String(chops) + " / " + String(CHOPS));
}

void resetDoor() {
  chops = 0;
  brokenAt = 0;
  setColor(0, 0, 0);
  showCount();
}

void chop() {
  if (brokenAt != 0) return;                           // ประตูพังแล้ว รอรีเซ็ต
  chops++;
  mqtt.publish(T_AXE.c_str(), String(chops).c_str());
  Serial.println("[TX] " + T_AXE + " : " + String(chops));
  showCount();
  setColor(255, 255, 255);                             // ไฟวาบตอนขวานกระทบ
  delay(40);
  setColor(0, 0, 0);

  if (chops >= CHOPS) {
    mqtt.publish(T_DOOR.c_str(), "Here's Johnny!");
    Serial.println("[TX] " + T_DOOR + " : Here's Johnny!");
    lcdLine(0, "Here's Johnny!");
    lcdLine(1, "Door is broken");
    brokenAt = millis();
  }
}

void messageReceived(String &topic, String &payload) {
  (void)topic;
  String msg = payload;
  msg.trim();
  msg.toLowerCase();
  Serial.println("[RX] " + msg);
  if (msg == "reset") resetDoor();
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
    if (mqtt.connect(id.c_str())) { mqtt.subscribe(T_CMD.c_str()); Serial.println("MQTT OK"); showCount(); }
    else { Serial.print("MQTT fail err="); Serial.print(mqtt.lastError()); Serial.print(" rc="); Serial.println(mqtt.returnCode()); delay(2000); }
  }
}

void setup() {
  Serial.begin(9600);
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  pinMode(R_PIN, OUTPUT); pinMode(G_PIN, OUTPUT); pinMode(B_PIN, OUTPUT);
  setColor(0, 0, 0);
  lcd.init();
  lcd.backlight();
  T_AXE  = STUDENT_ID + TOPIC_AXE;
  T_DOOR = STUDENT_ID + TOPIC_DOOR;
  T_CMD  = STUDENT_ID + TOPIC_CMD;
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

  // ปุ่ม: นับเฉพาะตอนกดลง และกันสัญญาณเด้ง 50 ms
  bool now = digitalRead(BUTTON_PIN);
  if (now != lastButton && millis() - tButton > 50) {
    tButton = millis();
    lastButton = now;
    if (now == LOW) chop();
  }

  // ประตูพัง: กระพริบแดง 3 วินาที แล้วเริ่มใหม่
  if (brokenAt != 0) {
    unsigned long t = millis() - brokenAt;
    if (t < 3000) setColor((t / 250) % 2 ? 255 : 0, 0, 0);
    else resetDoor();
  }
}
