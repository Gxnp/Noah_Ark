// =====================================================================
// BROKER_CHECK : ทดสอบว่า broker ตัวไหนใช้ได้กับ hotspot ของเรา (ใช้ 1 นาทีก่อนสอบ)
// บอร์ดจะลองทีละตัว: phycom.it.kmitl.ac.th, mqtt-dashboard.com, test.mosquitto.org
// แต่ละตัวทดสอบ 4 ขั้น
//   1) DNS   หา IP ของ host ได้ไหม
//   2) TCP   เปิดพอร์ต 1883 ได้ไหม
//   3) MQTT  connect สำเร็จไหม
//   4) LOOP  publish แล้ว subscribe กลับมาได้ภายใน 3 วินาทีไหม (วัดเวลาเป็น ms)
// ผลขึ้นที่ Serial Monitor (9600) และ LCD (ถ้าต่อไว้) แล้ววนทดสอบใหม่ทุก 30 วินาที
//
// บอร์ด Arduino UNO R4 WiFi   ไลบรารี: MQTT (MQTTClient.h), LiquidCrystal_I2C
// ต่อแค่ LCD I2C (ไม่ต่อก็ได้ ดูผลใน Serial Monitor)
// =====================================================================

#include <WiFiS3.h>
#include <MQTTClient.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

// ===== แก้ตรงนี้ =====
const char WIFI_SSID[]  = "iphone 80 promax";
const char WIFI_PASSWORD[]  = "88888888";
const String STUDENT_ID = "68070180";

const int NB = 3;
const char* HOSTS[NB]  = {"phycom.it.kmitl.ac.th", "mqtt-dashboard.com", "test.mosquitto.org"};
const char* SHORT[NB]  = {"phycom", "dashbrd", "mosquit"};
const int   MQTT_PORT  = 1883;

LiquidCrystal_I2C lcd(0x27, 16, 2);
WiFiClient network;
MQTTClient mqtt = MQTTClient(256);

String result[NB];
bool gotEcho = false;
String expected = "";

void lcdLine(int row, String s) {
  lcd.setCursor(0, row);
  while (s.length() < 16) s += ' ';
  lcd.print(s.substring(0, 16));
}

void messageReceived(String &topic, String &payload) {
  (void)topic;
  if (payload == expected) gotEcho = true;
}

void connectWiFi() {
  lcdLine(0, "WiFi...");
  lcdLine(1, WIFI_SSID);
  Serial.print("Connecting WiFi: ");
  Serial.println(WIFI_SSID);
  while (WiFi.begin(WIFI_SSID, WIFI_PASSWORD) != WL_CONNECTED) { Serial.print("."); delay(3000); }
  while (WiFi.localIP() == IPAddress(0, 0, 0, 0)) delay(200);
  Serial.print("WiFi OK, IP = ");
  Serial.print(WiFi.localIP());
  Serial.print("  RSSI = ");
  Serial.print(WiFi.RSSI());
  Serial.println(" dBm");
}

// คืนค่า: "OK 85ms" หรือชื่อขั้นที่ล้มเหลว
String testBroker(int i) {
  const char* host = HOSTS[i];
  Serial.println();
  Serial.print("=== ");
  Serial.print(host);
  Serial.println(" ===");
  lcdLine(0, String("Test ") + SHORT[i]);

  // 1) DNS
  lcdLine(1, "1 DNS...");
  IPAddress ip;
  if (WiFi.hostByName(host, ip) != 1 || ip == IPAddress(0, 0, 0, 0)) {
    Serial.println("  1 DNS   FAIL (หา IP ไม่ได้: hotspot ไม่มีเน็ต หรือชื่อผิด)");
    return "DNS fail";
  }
  Serial.print("  1 DNS   OK   ");
  Serial.println(ip);

  // 2) TCP
  lcdLine(1, "2 TCP 1883...");
  WiFiClient probe;
  if (!probe.connect(ip, MQTT_PORT)) {
    Serial.println("  2 TCP   FAIL (พอร์ต 1883 ถูกปิดกั้น)");
    return "TCP fail";
  }
  probe.stop();
  Serial.println("  2 TCP   OK");

  // 3) MQTT connect
  lcdLine(1, "3 MQTT...");
  mqtt.disconnect();
  mqtt.begin(host, MQTT_PORT, network);
  String id = "r4chk-" + STUDENT_ID + "-" + String(random(0xFFFF), HEX);
  if (!mqtt.connect(id.c_str())) {
    Serial.print("  3 MQTT  FAIL err=");
    Serial.print(mqtt.lastError()); Serial.print(" rc="); Serial.println(mqtt.returnCode());
    return "MQTT err=" + String(mqtt.lastError());
  }
  Serial.println("  3 MQTT  OK");

  // 4) ส่งแล้วรับกลับ
  lcdLine(1, "4 Pub/Sub...");
  String topic = STUDENT_ID + "/check";
  mqtt.subscribe(topic.c_str());
  for (int k = 0; k < 10; k++) { mqtt.loop(); delay(20); }       // ให้ subscribe เสร็จก่อน
  expected = "ping-" + String(millis());
  gotEcho = false;
  unsigned long t0 = millis();
  mqtt.publish(topic.c_str(), expected.c_str());
  while (!gotEcho && millis() - t0 < 3000) { mqtt.loop(); delay(5); }
  unsigned long dt = millis() - t0;
  mqtt.disconnect();
  if (!gotEcho) {
    Serial.println("  4 LOOP  FAIL (ส่งได้แต่ไม่ได้รับกลับใน 3 วินาที)");
    return "No echo";
  }
  Serial.print("  4 LOOP  OK   ");
  Serial.print(dt);
  Serial.println(" ms");
  return "OK " + String(dt) + "ms";
}

void runAll() {
  for (int i = 0; i < NB; i++) result[i] = testBroker(i);

  Serial.println();
  Serial.println("========== สรุป ==========");
  int best = -1;
  for (int i = 0; i < NB; i++) {
    Serial.print(HOSTS[i]);
    Serial.print("  ->  ");
    Serial.println(result[i]);
    if (best < 0 && result[i].startsWith("OK")) best = i;       // ใช้ตัวแรกที่ผ่านตามลำดับข้อสอบ
  }
  if (best >= 0) {
    Serial.print(">>> ใช้ MQTT_BROKER_ADRRESS = \"");
    Serial.print(HOSTS[best]);
    Serial.println("\"");
  } else {
    Serial.println(">>> ไม่มีตัวไหนผ่าน: เช็กเน็ตมือถือ / ลอง hotspot เครื่องอื่น");
  }

  // LCD แสดงผลทีละตัว ตัวละ 2 วินาที แล้วจบด้วยตัวที่แนะนำ
  for (int i = 0; i < NB; i++) {
    lcdLine(0, SHORT[i]);
    lcdLine(1, result[i]);
    delay(2000);
  }
  lcdLine(0, "Use:");
  lcdLine(1, best >= 0 ? HOSTS[best] : "none - check net");
}

void setup() {
  Serial.begin(9600);
  delay(1500);
  lcd.init();
  lcd.backlight();
  connectWiFi();
  randomSeed(micros());
  mqtt.onMessage(messageReceived);
  runAll();
}

void loop() {
  static unsigned long last = millis();
  if (millis() - last >= 30000) {          // ทดสอบซ้ำทุก 30 วินาที
    last = millis();
    if (WiFi.status() != WL_CONNECTED) connectWiFi();
    runAll();
  }
}
