// =====================================================================
// แนวข้อสอบ The Shining  S10 : "Forever, and ever, and ever" (เดา)
// subscribe student_id/countdown ได้ตัวเลขเริ่มต้น 1–9
//   7-Segment นับถอยหลังจากเลขนั้นถึง 0 ทุก 1 วินาที (ใช้ millis ไม่ใช้ delay)
//   ถึง 0 -> publish "Forever and ever" ไปที่ student_id/twins
//            Color LED ฟ้า (สีชุดฝาแฝด) แล้ววนนับใหม่จากเลขเดิม "ตลอดไป"
//   ได้ "stop" -> หยุดนับ แสดงขีดกลาง ไฟดับ
// ทุกครั้งที่ตัวเลขเปลี่ยน publish ค่าปัจจุบันไปที่ student_id/count ด้วย
//
// บอร์ด Arduino UNO R4 WiFi   ไลบรารี: MQTT (MQTTClient.h)
// วงจร 7-Segment (Common Cathode): a–g -> D2–D8 (R 330Ω), Common -> GND
//      Color LED (Common Anode): Common -> 5V, R -> D9, B -> D10, G -> D11 (R 330Ω)
// =====================================================================

#include <WiFiS3.h>
#include <MQTTClient.h>

// ===== แก้ตรงนี้ =====
const char WIFI_SSID[]  = "iphone 80 promax";
const char WIFI_PASSWORD[]  = "88888888";
const String STUDENT_ID = "68070180";
const char MQTT_BROKER_ADRRESS[] = "phycom.it.kmitl.ac.th";
const int   MQTT_PORT = 1883;
const String TOPIC_IN    = "/countdown";
const String TOPIC_COUNT = "/count";
const String TOPIC_TWINS = "/twins";
const unsigned long STEP_MS = 1000;

const int SEG_PINS[7] = {2, 3, 4, 5, 6, 7, 8};
const bool SEG_COMMON_ANODE = false;
const int R_PIN = 9, B_PIN = 10, G_PIN = 11;
const byte DIGITS[10][7] = {
  {1,1,1,1,1,1,0}, {0,1,1,0,0,0,0}, {1,1,0,1,1,0,1}, {1,1,1,1,0,0,1}, {0,1,1,0,0,1,1},
  {1,0,1,1,0,1,1}, {1,0,1,1,1,1,1}, {1,1,1,0,0,0,0}, {1,1,1,1,1,1,1}, {1,1,1,1,0,1,1}
};

WiFiClient network;
MQTTClient mqtt = MQTTClient(256);
String T_IN, T_COUNT, T_TWINS;

int startValue = 0;        // เลขเริ่มที่ได้รับ
int current = -1;          // -1 = ไม่ได้นับ
unsigned long lastStep = 0;
int rounds = 0;

void writeSeg(int i, bool on) { if (SEG_COMMON_ANODE) on = !on; digitalWrite(SEG_PINS[i], on ? HIGH : LOW); }
void showDigit(int n) { for (int i = 0; i < 7; i++) writeSeg(i, DIGITS[n][i]); }
void showDash()       { for (int i = 0; i < 7; i++) writeSeg(i, i == 6); }

void setColor(int r, int g, int b) {
  analogWrite(R_PIN, 255 - r);
  analogWrite(G_PIN, 255 - g);
  analogWrite(B_PIN, 255 - b);
}

bool countChanged = false;     // มีเลขใหม่รอ publish (ส่งใน loop เพราะห้าม publish ใน callback)

void showCount() {
  showDigit(current);
  countChanged = true;
  Serial.println("count " + String(current));
}

void messageReceived(String &topic, String &payload) {
  (void)topic;
  String msg = payload;
  msg.trim();
  msg.toLowerCase();
  Serial.println("[RX] " + msg);

  if (msg == "stop") { current = -1; showDash(); setColor(0, 0, 0); return; }
  int n = msg.toInt();
  if (n < 1 || n > 9) return;               // รับแค่ 1–9
  startValue = n;
  current = n;
  rounds = 0;
  lastStep = millis();
  setColor(0, 0, 0);
  showCount();
}

void connectWiFi() {
  while (WiFi.begin(WIFI_SSID, WIFI_PASSWORD) != WL_CONNECTED) { Serial.print("."); delay(3000); }
  while (WiFi.localIP() == IPAddress(0, 0, 0, 0)) delay(200);
  Serial.print("WiFi OK, IP = ");
  Serial.println(WiFi.localIP());
}

void connectToMQTT() {
  while (!mqtt.connected()) {
    String id = "r4-" + STUDENT_ID + "-" + String(random(0xFFFF), HEX);
    if (mqtt.connect(id.c_str())) { mqtt.subscribe(T_IN.c_str()); Serial.println("MQTT OK"); }
    else { Serial.print("MQTT fail err="); Serial.print(mqtt.lastError()); Serial.print(" rc="); Serial.println(mqtt.returnCode()); delay(2000); }
  }
}

void setup() {
  Serial.begin(9600);
  for (int i = 0; i < 7; i++) pinMode(SEG_PINS[i], OUTPUT);
  pinMode(R_PIN, OUTPUT); pinMode(G_PIN, OUTPUT); pinMode(B_PIN, OUTPUT);
  showDash();
  setColor(0, 0, 0);
  T_IN    = STUDENT_ID + TOPIC_IN;
  T_COUNT = STUDENT_ID + TOPIC_COUNT;
  T_TWINS = STUDENT_ID + TOPIC_TWINS;
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

  if (countChanged) {
    countChanged = false;
    mqtt.publish(T_COUNT.c_str(), String(current).c_str());
  }

  if (current < 0 || millis() - lastStep < STEP_MS) return;
  lastStep = millis();

  if (current > 0) {
    current--;
    if (current > 0) setColor(0, 0, 0);
    showCount();
    if (current == 0) {                     // ถึง 0
      rounds++;
      mqtt.publish(T_TWINS.c_str(), "Forever and ever");
      Serial.println("[TX] " + T_TWINS + " : Forever and ever (round " + String(rounds) + ")");
      setColor(0, 120, 255);                // ฟ้า
    }
  } else {                                  // อยู่ที่ 0 ครบ 1 วินาทีแล้ว วนใหม่
    current = startValue;
    setColor(0, 0, 0);
    showCount();
  }
}
