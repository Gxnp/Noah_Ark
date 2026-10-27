// =====================================================================
// เทมเพลตสอบ (แบบใช้ HiveMQ Browser Client: hivemq.com/demos/websocket-client)
//   บอร์ด -> publish ค่าเซนเซอร์ทุก 1 วินาที   -> TA/เรา subscribe ดูในหน้าเว็บ
//   หน้าเว็บ -> publish ข้อความ/คำสั่ง          -> บอร์ด subscribe แล้วแสดงบน LCD + LED + RGB
//   บอร์ดตอบกลับ "got: ..." และสถานะ online/offline ที่ TOPIC_STATUS ให้เห็นในหน้าเว็บ
//
// คำสั่งที่พิมพ์ในหน้าเว็บได้ (ตัวพิมพ์เล็กหรือใหญ่ก็ได้)
//   ข้อความอะไรก็ได้      -> แสดงบน LCD บรรทัดล่าง (ยาวกว่า 16 ตัวจะเลื่อน)
//   ON / OFF               -> LED สว่างสุด / ดับ (OFF ดับ RGB ด้วย)
//   ตัวเลข 0–255           -> ความสว่าง LED
//   RED GREEN BLUE YELLOW CYAN PURPLE WHITE -> สี RGB
//   CLEAR                  -> ล้างข้อความบนจอ
//
// บอร์ด Arduino UNO R4 WiFi   ไลบรารี: MQTT (MQTTClient.h), LiquidCrystal_I2C
// วงจร: Potentiometer ขากลาง -> A0 (ขาริม 5V, GND)
//       LED -> D3 (ผ่าน R 220Ω)
//       RGB Common Anode: Common -> 5V, R -> D9, B -> D10, G -> D11 (ผ่าน R 220Ω)
//       LCD I2C: SDA -> SDA, SCL -> SCL, VCC -> 5V, GND -> GND
// =====================================================================

#include <WiFiS3.h>
#include <MQTTClient.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

// ===== แก้ตรงนี้ =====
const char WIFI_SSID[]    = "iphone 80 promax";
const char WIFI_PASSWORD[]    = "88888888";
const char MQTT_BROKER_ADRRESS[]    = "mqtt-dashboard.com";      // broker เดียวกับหน้า HiveMQ Client
const int   MQTT_PORT    = 1883;                     // บอร์ดใช้ 1883 / หน้าเว็บใช้ 8884
const char* TOPIC_SENSOR = "68070180/sensor";   // บอร์ดส่ง  -> subscribe ในหน้าเว็บ
const char* TOPIC_CMD    = "68070180/cmd";      // บอร์ดรับ  <- publish จากหน้าเว็บ
const char* TOPIC_STATUS = "68070180/status";   // online / offline / got: ...

const int POT_PIN = A0;
const int LED_PIN = 3;
const int R_PIN = 9, B_PIN = 10, G_PIN = 11;

LiquidCrystal_I2C lcd(0x27, 16, 2);   // ถ้าจอไม่ขึ้น ลอง 0x3F
WiFiClient network;
MQTTClient mqtt = MQTTClient(512);

unsigned long lastPub = 0;
String pendingAck = "";        // ข้อความตอบกลับที่รอส่งใน loop()
String shownMsg = "";          // ข้อความที่แสดงบรรทัดล่าง
unsigned int scrollPos = 0;
unsigned long lastScroll = 0;

// ---------------------------------------------------------------------
// อ่านค่าเซนเซอร์ (เปลี่ยนได้ ก๊อปฟังก์ชันจาก EXAM_Sensor_MQTT_Display)
// ---------------------------------------------------------------------
String readSensor() {
  return String(analogRead(POT_PIN));   // 0–1023
}

// ---------------------------------------------------------------------
// แสดงผล
// ---------------------------------------------------------------------
void lcdLine(int row, String s) {
  lcd.setCursor(0, row);
  while (s.length() < 16) s += ' ';
  lcd.print(s.substring(0, 16));
}

void setRgb(bool r, bool g, bool b) {    // Common Anode: LOW = ติด
  digitalWrite(R_PIN, r ? LOW : HIGH);
  digitalWrite(G_PIN, g ? LOW : HIGH);
  digitalWrite(B_PIN, b ? LOW : HIGH);
}

bool isNumber(const String& s) {
  if (s.length() == 0) return false;
  for (unsigned int i = 0; i < s.length(); i++) if (!isDigit(s[i])) return false;
  return true;
}

void showMessage(String s) {
  shownMsg = s;
  scrollPos = 0;
  lastScroll = millis();
  lcdLine(1, shownMsg);
}

// ข้อความยาวกว่า 16 ตัวให้เลื่อนทีละตัว (ไม่ใช้ delay)
void updateScroll() {
  if (shownMsg.length() <= 16 || millis() - lastScroll < 400) return;
  lastScroll = millis();
  String loopText = shownMsg + "   ";
  scrollPos = (scrollPos + 1) % loopText.length();
  String view = (loopText + loopText).substring(scrollPos, scrollPos + 16);
  lcdLine(1, view);
}

void messageReceived(String &topic, String &payload) {
  String msg = payload;
  msg.trim();                          // ตัดช่องว่าง/ขึ้นบรรทัดที่พิมพ์เกินมา
  String cmd = msg;
  cmd.toUpperCase();

  Serial.print("[RX] ");
  Serial.print(topic);
  Serial.print(" : ");
  Serial.println(msg);

  if (cmd == "ON")             analogWrite(LED_PIN, 255);
  else if (cmd == "OFF")     { analogWrite(LED_PIN, 0); setRgb(0, 0, 0); }
  else if (isNumber(cmd))      analogWrite(LED_PIN, constrain(cmd.toInt(), 0, 255));
  else if (cmd == "RED")       setRgb(1, 0, 0);
  else if (cmd == "GREEN")     setRgb(0, 1, 0);
  else if (cmd == "BLUE")      setRgb(0, 0, 1);
  else if (cmd == "YELLOW")    setRgb(1, 1, 0);
  else if (cmd == "CYAN")      setRgb(0, 1, 1);
  else if (cmd == "PURPLE" || cmd == "MAGENTA") setRgb(1, 0, 1);
  else if (cmd == "WHITE")     setRgb(1, 1, 1);

  if (cmd == "CLEAR") showMessage("");
  else                showMessage(msg);

  pendingAck = "got: " + msg;            // ห้าม publish ใน callback ของ MQTTClient เก็บไว้ส่งใน loop()
}

// ---------------------------------------------------------------------
// WiFi + MQTT
// ---------------------------------------------------------------------
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
    String clientId = "r4-exam-" + String(random(0xFFFFFF), HEX);
    // Last Will: ถ้าบอร์ดหลุด broker จะส่ง "offline" ให้เองที่ TOPIC_STATUS
    mqtt.setWill(TOPIC_STATUS, "offline", true, 0);
    if (mqtt.connect(clientId.c_str())) {
      mqtt.publish(TOPIC_STATUS, "online", true, 0);
      mqtt.subscribe(TOPIC_CMD);
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
  mqtt.loop();
  if (pendingAck.length() > 0) {         // ส่งคำตอบกลับหลัง mqtt.loop()
    mqtt.publish(TOPIC_STATUS, pendingAck.c_str());
    pendingAck = "";
  }
  updateScroll();

  if (millis() - lastPub >= 1000) {
    lastPub = millis();
    String value = readSensor();
    mqtt.publish(TOPIC_SENSOR, value.c_str());
    Serial.print("[TX] ");
    Serial.print(TOPIC_SENSOR);
    Serial.print(" : ");
    Serial.println(value);
    lcdLine(0, "TX: " + value);
  }
}
