// =====================================================================
// แนวข้อสอบ The Shining  S7 : Keypad ไขประตูห้อง 237 (เดา · ถ้าปีนี้มี Keypad)
// พิมพ์เลขห้องบน Keypad แล้วกด #  (กด * ลบ)
//   ทุกครั้งที่กด #     -> publish เลขที่พิมพ์ไปที่ student_id/room_code
//   เลขตรง SECRET (237) -> publish "OPEN" ไปที่ student_id/room237
//                          LCD "Room 237" / "Door opened"  Color LED แดง
//   เลขไม่ตรง          -> LCD "Room xxx" / "Wrong room"     Color LED เขียว
//   กด A               -> publish "REDRUM" ไปที่ student_id/message  LCD แสดง "REDRUM" กับ "MURDER" (กระจกสะท้อน)
// subscribe student_id/room_msg แล้วแสดงข้อความบน LCD บรรทัด 2
//
// บอร์ด Arduino UNO R4 WiFi   ไลบรารี: MQTT (MQTTClient.h), Adafruit Keypad, LiquidCrystal_I2C
// วงจร Keypad 4x4: R1–R4 -> D9, D8, D7, D6   C1–C4 -> D5, D4, D3, D2
//      Color LED (Common Anode): Common -> 5V, R -> D10, G -> D11, B -> D12 (R 330Ω)
//      LCD I2C: SDA, SCL, 5V, GND
// =====================================================================

#include <WiFiS3.h>
#include <MQTTClient.h>
#include "Adafruit_Keypad.h"
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

// ===== แก้ตรงนี้ =====
const char WIFI_SSID[]  = "iphone 80 promax";
const char WIFI_PASSWORD[]  = "88888888";
const String STUDENT_ID = "68070180";
const char MQTT_BROKER_ADRRESS[] = "phycom.it.kmitl.ac.th";
const int   MQTT_PORT = 1883;
const String SECRET = "237";

const int R_PIN = 10, G_PIN = 11, B_PIN = 12;          // D12 ไม่ใช่ PWM จึงใช้แค่เปิด/ปิด

const byte ROWS = 4, COLS = 4;
char keys[ROWS][COLS] = {
  {'1','2','3','A'}, {'4','5','6','B'}, {'7','8','9','C'}, {'*','0','#','D'}
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
String T_CODE, T_ROOM, T_MSG, T_IN;
String input = "";

void lcdLine(int row, String s) {
  lcd.setCursor(0, row);
  while (s.length() < 16) s += ' ';
  lcd.print(s.substring(0, 16));
}

void setRgb(bool r, bool g, bool b) {                  // Common Anode: LOW = ติด
  digitalWrite(R_PIN, r ? LOW : HIGH);
  digitalWrite(G_PIN, g ? LOW : HIGH);
  digitalWrite(B_PIN, b ? LOW : HIGH);
}

void messageReceived(String &topic, String &payload) {
  (void)topic;
  String msg = payload;
  msg.trim();
  Serial.println("[RX] " + msg);
  lcdLine(1, msg);
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
      lcdLine(0, "Room: ");
    } else { Serial.print("MQTT fail err="); Serial.print(mqtt.lastError()); Serial.print(" rc="); Serial.println(mqtt.returnCode()); delay(2000); }
  }
}

void setup() {
  Serial.begin(9600);
  keypad.begin();
  pinMode(R_PIN, OUTPUT); pinMode(G_PIN, OUTPUT); pinMode(B_PIN, OUTPUT);
  setRgb(0, 0, 0);
  lcd.init();
  lcd.backlight();
  T_CODE = STUDENT_ID + "/room_code";
  T_ROOM = STUDENT_ID + "/room237";
  T_MSG  = STUDENT_ID + "/message";
  T_IN   = STUDENT_ID + "/room_msg";
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

  char k = getKey();
  if (!k) return;

  if (k >= '0' && k <= '9') {
    if (input.length() < 4) input += k;
    lcdLine(0, "Room: " + input);
  } else if (k == '*') {
    input = "";
    lcdLine(0, "Room: ");
    lcdLine(1, "");
    setRgb(0, 0, 0);
  } else if (k == '#' && input.length() > 0) {
    mqtt.publish(T_CODE.c_str(), input.c_str());
    Serial.println("[TX] " + T_CODE + " : " + input);
    if (input == SECRET) {
      mqtt.publish(T_ROOM.c_str(), "OPEN");
      lcdLine(0, "Room 237");
      lcdLine(1, "Door opened");
      setRgb(1, 0, 0);
    } else {
      lcdLine(0, "Room " + input);
      lcdLine(1, "Wrong room");
      setRgb(0, 1, 0);
    }
    input = "";
  } else if (k == 'A') {
    mqtt.publish(T_MSG.c_str(), "REDRUM");
    lcdLine(0, "REDRUM");
    lcdLine(1, "MURDER");                               // REDRUM ในกระจก
    setRgb(1, 0, 0);
  }
}
