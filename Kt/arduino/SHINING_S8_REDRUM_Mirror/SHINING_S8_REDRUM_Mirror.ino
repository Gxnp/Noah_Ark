// =====================================================================
// แนวข้อสอบ The Shining  S8 : REDRUM ในกระจก (เดา)
// Danny เขียน "REDRUM" บนประตู เมื่อมองผ่านกระจกจะอ่านได้ว่า "MURDER"
// subscribe student_id/redrum แล้ว
//   LCD บรรทัด 1: ข้อความที่ได้รับ        เช่น "REDRUM"
//   LCD บรรทัด 2: ข้อความกลับหลัง (กระจก) เช่น "MURDER"
//   publish ข้อความกลับหลังไปที่ student_id/mirror
//   ถ้าคำกลับหลังเป็น "MURDER" -> Color LED แดงกระพริบ 3 วินาที
// กดปุ่มที่ A2 = ส่ง "REDRUM" เองเพื่อทดสอบ
//
// บอร์ด Arduino UNO R4 WiFi   ไลบรารี: MQTT (MQTTClient.h), LiquidCrystal_I2C
// วงจร LCD I2C: SDA, SCL, 5V, GND
//      Color LED (Common Anode): Common -> 5V, R -> D9, B -> D10, G -> D11 (R 330Ω)
//      Push Button: A2 -> ปุ่ม -> GND
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
const String TOPIC_IN  = "/redrum";
const String TOPIC_OUT = "/mirror";

const int R_PIN = 9, B_PIN = 10, G_PIN = 11;
const int BUTTON_PIN = A2;

LiquidCrystal_I2C lcd(0x27, 16, 2);
WiFiClient network;
MQTTClient mqtt = MQTTClient(256);
String T_IN, T_OUT;
unsigned long alarmAt = 0;
String pendingMirror = "";
bool lastButton = HIGH;
unsigned long tButton = 0;

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

// กลับหลังข้อความ: "REDRUM" -> "MURDER"
String reverseText(String s) {
  String out = "";
  for (int i = s.length() - 1; i >= 0; i--) out += s[i];
  return out;
}

void messageReceived(String &topic, String &payload) {
  (void)topic;
  String msg = payload;
  msg.trim();

  String mirror = reverseText(msg);
  lcdLine(0, msg);
  lcdLine(1, mirror);
  pendingMirror = mirror;                               // ส่งใน loop() (ห้าม publish ใน callback)
  Serial.println("[RX] " + msg + "  -> " + mirror);

  String check = mirror;
  check.toUpperCase();
  if (check == "MURDER") alarmAt = millis();            // เริ่มกระพริบแดง
  else setColor(0, 0, 0);
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
      mqtt.subscribe(T_IN.c_str());
      Serial.println("MQTT OK");
      lcdLine(0, "Waiting word..");
      lcdLine(1, "");
    } else { Serial.print("MQTT fail err="); Serial.print(mqtt.lastError()); Serial.print(" rc="); Serial.println(mqtt.returnCode()); delay(2000); }
  }
}

void setup() {
  Serial.begin(9600);
  pinMode(R_PIN, OUTPUT); pinMode(G_PIN, OUTPUT); pinMode(B_PIN, OUTPUT);
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  setColor(0, 0, 0);
  lcd.init();
  lcd.backlight();
  T_IN  = STUDENT_ID + TOPIC_IN;
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

  if (pendingMirror.length() > 0) {
    mqtt.publish(T_OUT.c_str(), pendingMirror.c_str());
    Serial.println("[TX] " + T_OUT + " : " + pendingMirror);
    pendingMirror = "";
  }

  // ปุ่มทดสอบ: ส่ง REDRUM เข้า topic ตัวเอง
  bool now = digitalRead(BUTTON_PIN);
  if (now != lastButton && millis() - tButton > 50) {
    tButton = millis();
    lastButton = now;
    if (now == LOW) mqtt.publish(T_IN.c_str(), "REDRUM");
  }

  // กระพริบแดง 3 วินาที
  if (alarmAt != 0) {
    unsigned long t = millis() - alarmAt;
    if (t < 3000) setColor((t / 200) % 2 ? 255 : 0, 0, 0);
    else { setColor(0, 0, 0); alarmAt = 0; }
  }
}
