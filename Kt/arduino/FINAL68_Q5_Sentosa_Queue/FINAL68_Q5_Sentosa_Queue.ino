// =====================================================================
// Final 68 ข้อ 5 : ภารกิจพิชิตคิวที่ Sentosa (4 คะแนน)
// subscribe เวลารอคิว (นาที) 3 แห่งที่เพื่อนส่งมาทุก 1–2 วินาที
//   student_id/queue_chotiphat  = Universal Studios
//   student_id/queue_lapas      = S.E.A. Aquarium
//   student_id/queue_sumet      = Skyline Luge
// แล้ว
//   1) publish ชื่อสถานที่ที่คิวน้อยที่สุด ("Universal" / "Aquarium" / "Luge") ไปที่ student_id/sentosa_choice
//   2) หาค่าเฉลี่ยของทั้ง 3 แห่ง
//   3) LCD  บรรทัด 1: สถานที่ที่คิวน้อยสุด + เวลา  เช่น "Luge (15m)"
//           บรรทัด 2: ค่าเฉลี่ย                    เช่น "Avg: 45m"
//   4) Color LED ตามค่าเฉลี่ย  > 60 นาที = แดง,  30–60 = ส้ม,  < 30 = เขียวอมฟ้า
//
// บอร์ด Arduino UNO R4 WiFi   ไลบรารี: MQTT (MQTTClient.h), LiquidCrystal_I2C
// วงจร Color LED (Common Anode): Common -> 5V, R -> D9, B -> D10, G -> D11 (ผ่าน R 330Ω)
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

const int R_PIN = 9, B_PIN = 10, G_PIN = 11;     // ขา PWM ทั้ง 3 ขา (ต้องผสมสีส้ม)
const bool COMMON_ANODE = true;                  // Color LED ที่แจกเป็น Common Anode

LiquidCrystal_I2C lcd(0x27, 16, 2);
WiFiClient network;
MQTTClient mqtt = MQTTClient(256);

// ข้อมูล 3 สถานที่ (ลำดับเดียวกันทุก array)
const char* PLACE_NAME[3]  = {"Universal", "Aquarium", "Luge"};
const char* TOPIC_SUFFIX[3] = {"/queue_chotiphat", "/queue_lapas", "/queue_sumet"};
float queueMin[3] = {-1, -1, -1};                // -1 = ยังไม่ได้รับค่า
bool  updated = false;
unsigned long lastCalc = 0;
String TOPIC_CHOICE;

void lcdLine(int row, String s) {
  lcd.setCursor(0, row);
  while (s.length() < 16) s += ' ';
  lcd.print(s.substring(0, 16));
}

// สี 0–255 ต่อช่อง (Common Anode ต้องกลับค่า 255 - v)
void setColor(int r, int g, int b) {
  if (COMMON_ANODE) { r = 255 - r; g = 255 - g; b = 255 - b; }
  analogWrite(R_PIN, r);
  analogWrite(G_PIN, g);
  analogWrite(B_PIN, b);
}

void messageReceived(String &topic, String &payload) {
  String t = topic;
  String msg = payload;
  msg.trim();

  for (int i = 0; i < 3; i++) {
    if (t.endsWith(TOPIC_SUFFIX[i])) {
      queueMin[i] = msg.toFloat();               // รับได้ทั้ง "45" "45.5" และ "45m"
      updated = true;
      Serial.print("[RX] ");
      Serial.print(PLACE_NAME[i]);
      Serial.print(" = ");
      Serial.println(queueMin[i]);
    }
  }
}

void processQueues() {
  for (int i = 0; i < 3; i++) {
    if (queueMin[i] < 0) {                       // ยังรับไม่ครบ 3 แห่ง
      lcdLine(0, "Waiting queue..");
      lcdLine(1, String(PLACE_NAME[i]) + " ?");
      return;
    }
  }

  // 1) หาที่คิวน้อยสุด
  int best = 0;
  for (int i = 1; i < 3; i++) if (queueMin[i] < queueMin[best]) best = i;

  // 2) ค่าเฉลี่ย
  float avg = (queueMin[0] + queueMin[1] + queueMin[2]) / 3.0;

  // publish ชื่อสถานที่
  mqtt.publish(TOPIC_CHOICE.c_str(), PLACE_NAME[best]);

  // 3) LCD
  int bestMin = (int)round(queueMin[best]);
  int avgMin  = (int)round(avg);
  lcdLine(0, String(PLACE_NAME[best]) + " (" + String(bestMin) + "m)");
  lcdLine(1, "Avg: " + String(avgMin) + "m");

  // 4) Color LED ตามค่าเฉลี่ย
  if (avg > 60)        setColor(255, 0, 0);      // แดง        คนเยอะมาก
  else if (avg >= 30)  setColor(255, 70, 0);     // ส้ม        คนปานกลาง
  else                 setColor(0, 255, 120);    // เขียวอมฟ้า  คิวโล่ง

  Serial.print("[TX] ");
  Serial.print(TOPIC_CHOICE);
  Serial.print(" : ");
  Serial.print(PLACE_NAME[best]);
  Serial.print("   avg = ");
  Serial.println(avg, 1);
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
      for (int i = 0; i < 3; i++) {
        String topic = STUDENT_ID + TOPIC_SUFFIX[i];
        mqtt.subscribe(topic.c_str());
        Serial.println("sub " + topic);
      }
      Serial.println("MQTT OK");
      lcdLine(0, "Waiting queue..");
      lcdLine(1, "");
    } else {
      Serial.print("MQTT fail err=");
      Serial.print(mqtt.lastError()); Serial.print(" rc="); Serial.println(mqtt.returnCode());
      delay(2000);
    }
  }
}

void setup() {
  Serial.begin(9600);
  pinMode(R_PIN, OUTPUT);
  pinMode(G_PIN, OUTPUT);
  pinMode(B_PIN, OUTPUT);
  setColor(0, 0, 0);
  lcd.init();
  lcd.backlight();
  TOPIC_CHOICE = STUDENT_ID + "/sentosa_choice";
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

  // คำนวณใหม่เมื่อมีค่าเข้ามา แต่ไม่ถี่กว่า 0.5 วินาที
  if (updated && millis() - lastCalc >= 500) {
    updated = false;
    lastCalc = millis();
    processQueues();
  }
}
