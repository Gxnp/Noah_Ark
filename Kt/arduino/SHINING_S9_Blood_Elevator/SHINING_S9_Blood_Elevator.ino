// =====================================================================
// แนวข้อสอบ The Shining  S9 : ลิฟต์ที่เลือดทะลัก (เดา)
// หมุน Potentiometer = ระดับเลือด 0–100%
//   publish ระดับ (%) ไปที่ student_id/blood
//   subscribe student_id/blood กลับมา แล้ว
//     Color LED แดงสว่างตามระดับ (fade ด้วย PWM)
//     LCD บรรทัด 1 "Blood: 64%"   บรรทัด 2 แถบ ##########
//     ระดับ >= 80% -> LCD บรรทัด 1 เปลี่ยนเป็น "ELEVATOR FLOOD!"
//
// บอร์ด Arduino UNO R4 WiFi   ไลบรารี: MQTT (MQTTClient.h), LiquidCrystal_I2C
// วงจร Potentiometer: ขากลาง -> A0 (ขาริม 5V, GND)
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
const String TOPIC_NAME = "/blood";
const int FLOOD_LEVEL = 80;

const int POT_PIN = A0;
const int R_PIN = 9, B_PIN = 10, G_PIN = 11;

LiquidCrystal_I2C lcd(0x27, 16, 2);
WiFiClient network;
MQTTClient mqtt = MQTTClient(256);
String TOPIC;
int lastSent = -1;
unsigned long lastPub = 0;

void lcdLine(int row, String s) {
  lcd.setCursor(0, row);
  while (s.length() < 16) s += ' ';
  lcd.print(s.substring(0, 16));
}

void setColor(int r, int g, int b) {                    // Common Anode
  analogWrite(R_PIN, 255 - r);
  analogWrite(G_PIN, 255 - g);
  analogWrite(B_PIN, 255 - b);
}

void messageReceived(String &topic, String &payload) {
  (void)topic;
  String msg = payload;
  int level = constrain(msg.toInt(), 0, 100);

  setColor(map(level, 0, 100, 0, 255), 0, 0);           // แดงเข้มขึ้นตามระดับ

  String bar = "";
  for (int i = 0; i < map(level, 0, 100, 0, 16); i++) bar += '#';   // แถบ 16 ช่อง
  lcdLine(0, level >= FLOOD_LEVEL ? "ELEVATOR FLOOD!" : "Blood: " + String(level) + "%");
  lcdLine(1, bar);
  Serial.println("[RX] blood = " + String(level));
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
    if (mqtt.connect(id.c_str())) { mqtt.subscribe(TOPIC.c_str()); Serial.println("MQTT OK"); }
    else { Serial.print("MQTT fail err="); Serial.print(mqtt.lastError()); Serial.print(" rc="); Serial.println(mqtt.returnCode()); delay(2000); }
  }
}

void setup() {
  Serial.begin(9600);
  pinMode(R_PIN, OUTPUT); pinMode(G_PIN, OUTPUT); pinMode(B_PIN, OUTPUT);
  setColor(0, 0, 0);
  lcd.init();
  lcd.backlight();
  TOPIC = STUDENT_ID + TOPIC_NAME;
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

  int level = map(analogRead(POT_PIN), 0, 1023, 0, 100);
  if ((level != lastSent || millis() - lastPub >= 2000) && millis() - lastPub >= 150) {
    mqtt.publish(TOPIC.c_str(), String(level).c_str());
    Serial.println("[TX] " + TOPIC + " : " + String(level));
    lastSent = level;
    lastPub = millis();
  }
}
