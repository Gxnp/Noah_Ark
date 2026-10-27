// แนวข้อสอบ 4 : ระบบรหัสผ่าน
// พิมพ์รหัสบน Keypad แล้วกด # -> publish  |  subscribe -> ตรวจรหัส
//   ถูก = LCD "ACCESS GRANTED" + RGB เขียว   ผิด = "ACCESS DENIED" + RGB แดง (ค้าง 3 วินาที)
// กด * = ลบที่พิมพ์
// บอร์ด Arduino UNO R4 WiFi   ไลบรารี: MQTT (MQTTClient.h), Adafruit Keypad, LiquidCrystal_I2C
// วงจร Keypad: R1–R4 -> D9, D8, D7, D6   C1–C4 -> D5, D4, D3, D2
// วงจร RGB (Common Anode): Common -> 5V, R -> D10, G -> D11, B -> D12 (ผ่าน R 220Ω)
// วงจร LCD: SDA -> SDA, SCL -> SCL, VCC -> 5V, GND -> GND

#include <WiFiS3.h>
#include <MQTTClient.h>
#include "Adafruit_Keypad.h"
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

// ===== แก้ตรงนี้ =====
const char WIFI_SSID[] = "iphone 80 promax";
const char WIFI_PASSWORD[] = "88888888";
const char MQTT_BROKER_ADRRESS[] = "mqtt-dashboard.com";
const int   MQTT_PORT = 1883;
const char* TOPIC_PUB = "68070180/password";
const char* TOPIC_SUB = "68070180/password";
const String PASSWORD = "1234";          // รหัสที่ถูกต้อง (เช่น 4 ตัวท้ายรหัสนักศึกษา)

const int R_PIN = 10, G_PIN = 11, B_PIN = 12;

const byte ROWS = 4, COLS = 4;
char keys[ROWS][COLS] = {
  {'1', '2', '3', 'A'},
  {'4', '5', '6', 'B'},
  {'7', '8', '9', 'C'},
  {'*', '0', '#', 'D'}
};
byte rowPins[ROWS] = {9, 8, 7, 6};
byte colPins[COLS] = {5, 4, 3, 2};
Adafruit_Keypad keypad = Adafruit_Keypad(makeKeymap(keys), rowPins, colPins, ROWS, COLS);
// อ่านปุ่ม Keypad แบบไลบรารี Adafruit_Keypad (แบบเดียวกับโค้ดของวิชา) คืน 0 ถ้าไม่มีการกด
char getKey() {
  keypad.tick();
  while (keypad.available()) {
    keypadEvent e = keypad.read();
    if (e.bit.EVENT == KEY_JUST_PRESSED) return (char)e.bit.KEY;
  }
  return 0;
}


LiquidCrystal_I2C lcd(0x27, 16, 2);
WiFiClient network;
MQTTClient mqtt = MQTTClient(256);
String input = "";
unsigned long resultAt = 0;     // เวลาที่แสดงผลล่าสุด ใช้ดับไฟหลัง 3 วินาที

void lcdLine(int row, String s) {
  lcd.setCursor(0, row);
  while (s.length() < 16) s += ' ';
  lcd.print(s.substring(0, 16));
}

void setRgb(bool r, bool g, bool b) {     // Common Anode: LOW = ติด
  digitalWrite(R_PIN, r ? LOW : HIGH);
  digitalWrite(G_PIN, g ? LOW : HIGH);
  digitalWrite(B_PIN, b ? LOW : HIGH);
}

void showInput() {
  String stars = "";
  for (unsigned int i = 0; i < input.length(); i++) stars += '*';   // ซ่อนรหัสด้วย *
  lcdLine(0, "Pass: " + stars);
}

void messageReceived(String &topic, String &payload) {
  (void)topic;
  String msg = payload;
  msg.trim();                          // ตัดช่องว่างที่อาจพิมพ์เกินมาจากหน้า HiveMQ
  Serial.print("[RX] ");
  Serial.println(msg);

  if (msg == PASSWORD) {
    lcdLine(1, "ACCESS GRANTED");
    setRgb(0, 1, 0);
  } else {
    lcdLine(1, "ACCESS DENIED");
    setRgb(1, 0, 0);
  }
  resultAt = millis();
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
    String clientId = "r4-g4-" + String(random(0xFFFFFF), HEX);
    if (mqtt.connect(clientId.c_str())) {
      mqtt.subscribe(TOPIC_SUB);
      showInput();
    } else {
      Serial.print("MQTT fail err=");
      Serial.print(mqtt.lastError()); Serial.print(" rc="); Serial.println(mqtt.returnCode());
      delay(2000);
    }
  }
}

void setup() {
  Serial.begin(9600);
  keypad.begin();
  pinMode(R_PIN, OUTPUT);
  pinMode(G_PIN, OUTPUT);
  pinMode(B_PIN, OUTPUT);
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

  // ดับไฟและล้างผลหลัง 3 วินาที (ไม่ใช้ delay เพื่อให้ mqtt.loop ทำงานตลอด)
  if (resultAt != 0 && millis() - resultAt > 3000) {
    setRgb(0, 0, 0);
    lcdLine(1, "");
    resultAt = 0;
  }

  char key = getKey();
  if (!key) return;

  if (key == '*') {
    input = "";
  } else if (key == '#') {
    if (input.length() > 0) {
      mqtt.publish(TOPIC_PUB, input.c_str());
      Serial.print("[TX] ");
      Serial.println(input);
      input = "";
    }
  } else if (input.length() < 8) {
    input += key;
  }
  showInput();
}
