// โพยโค้ดสั้น: รวมชิ้นโค้ดที่ก๊อปไปวางได้ทันที (ไฟล์นี้คอมไพล์ผ่านทั้งไฟล์บน UNO R4 WiFi)
// แต่ละชิ้นเริ่มด้วยบรรทัด "// @@" ก๊อปตั้งแต่บรรทัดถัดไปจนถึง "// @@" ถัดไป
// บรรทัดที่ขึ้นต้น "// setup:" คือสิ่งที่ต้องใส่เพิ่มใน setup()

// @@hidden
#include <WiFiS3.h>
#include <MQTTClient.h>
WiFiClient network;
MQTTClient mqtt = MQTTClient(256);
const char* TOPIC_PUB = "68070180/sensor";

// @@ read:pot | Potentiometer | ค่า 0–1023 · ขากลาง → A0
const int POT_PIN = A0;

int readPot() {
  return analogRead(POT_PIN);            // 0–1023
}
// เป็นเปอร์เซ็นต์: map(readPot(), 0, 1023, 0, 100)

// @@ read:ultra | Ultrasonic HC-SR04 | ระยะ cm · Trig D4, Echo D2
const int TRIG_PIN = 4, ECHO_PIN = 2;
// setup: pinMode(TRIG_PIN, OUTPUT); pinMode(ECHO_PIN, INPUT);

float readDistanceCm() {
  digitalWrite(TRIG_PIN, LOW);  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH); delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);
  long us = pulseIn(ECHO_PIN, HIGH, 30000);   // รอสูงสุด 30 ms
  if (us == 0) return -1;                     // ไม่มีเสียงสะท้อน
  return us / 29.0 / 2.0;
}

// @@ read:mcp | MCP9700 | °C · Vout → A1
const int MCP_PIN = A1;

float readMcp9700C() {
  float v = analogRead(MCP_PIN) * 5.0 / 1023.0;
  return (v - 0.5) / 0.01;               // 0.5 V = 0 °C, 10 mV/°C
}

// @@ read:therm | Thermistor + R 10kΩ | °C · 5V → R10k → A2 → NTC → GND
const int THERM_PIN = A2;

float readThermistorC() {
  int raw = analogRead(THERM_PIN);
  if (raw <= 0 || raw >= 1023) return NAN;    // สายหลุด
  float r = 10000.0 * raw / (1023.0 - raw);   // ความต้านทาน NTC
  float tK = 1.0 / (1.0 / 298.15 + log(r / 10000.0) / 4050.0);
  return tK - 273.15;
}

// @@ read:button | ปุ่มกด | 1 = กด · D7 → ปุ่ม → GND
const int BUTTON_PIN = 7;
// setup: pinMode(BUTTON_PIN, INPUT_PULLUP);   ไม่ต้องมีตัวต้านทาน

bool isPressed() {
  return digitalRead(BUTTON_PIN) == LOW;      // PULLUP: กด = LOW
}

// @@ read:keypad | Keypad 4x4 (Adafruit_Keypad) | ทีละปุ่ม หรือพิมพ์แล้วกด # · D9–D2
#include "Adafruit_Keypad.h"
const byte ROWS = 4, COLS = 4;
char keys[ROWS][COLS] = {
  {'1','2','3','A'}, {'4','5','6','B'}, {'7','8','9','C'}, {'*','0','#','D'}
};
byte rowPins[ROWS] = {9, 8, 7, 6};
byte colPins[COLS] = {5, 4, 3, 2};
Adafruit_Keypad keypad = Adafruit_Keypad(makeKeymap(keys), rowPins, colPins, ROWS, COLS);
// setup: keypad.begin();
// อ่านปุ่ม Keypad แบบไลบรารี Adafruit_Keypad (แบบเดียวกับโค้ดของวิชา) คืน 0 ถ้าไม่มีการกด
char getKey() {
  keypad.tick();
  while (keypad.available()) {
    keypadEvent e = keypad.read();
    if (e.bit.EVENT == KEY_JUST_PRESSED) return (char)e.bit.KEY;
  }
  return 0;
}

String keyBuffer = "";

// ทีละปุ่ม: char k = getKey(); if (k) { ... }
// พิมพ์หลายตัวแล้วกด # : คืนข้อความตอนกด # ไม่งั้นคืน ""  (* = ลบ)
String readKeypadLine() {
  char k = getKey();
  if (!k) return "";
  if (k == '*') { keyBuffer = ""; return ""; }
  if (k == '#') { String out = keyBuffer; keyBuffer = ""; return out; }
  keyBuffer += k;
  return "";
}

// @@ show:lcd | LCD 16x2 I2C | SDA, SCL, 5V, GND
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
LiquidCrystal_I2C lcd(0x27, 16, 2);      // ไม่ขึ้นลอง 0x3F
// setup: lcd.init(); lcd.backlight();

void lcdLine(int row, String s) {        // เขียนทับทั้งบรรทัด ไม่ต้อง lcd.clear()
  lcd.setCursor(0, row);
  while (s.length() < 16) s += ' ';
  lcd.print(s.substring(0, 16));
}
// องศา: lcdLine(0, "T:" + String(25.4, 1) + (char)223 + "C");

// @@ show:i2c | หา address ของจอ LCD | พิมพ์ทาง Serial
void i2cScan() {                         // เรียกใน setup() หลัง Serial.begin
  Wire.begin();
  for (byte a = 1; a < 127; a++) {
    Wire.beginTransmission(a);
    if (Wire.endTransmission() == 0) {
      Serial.print("I2C found 0x");
      Serial.println(a, HEX);            // ส่วนใหญ่ 0x27 หรือ 0x3F
    }
  }
}

// @@ show:led | LED หรี่ (PWM) | ขา ~ : 3 5 6 9 10 11
const int LED_PIN = 3;
// setup: pinMode(LED_PIN, OUTPUT);

void setLed(int v) {                     // 0 = ดับ, 255 = สว่างสุด
  analogWrite(LED_PIN, constrain(v, 0, 255));
}
// จากช่วงอื่น: setLed(map(value, 0, 1023, 0, 255));

// @@ show:rgb | RGB Common Anode | Common → 5V · R D9, B D10, G D11
const int R_PIN = 9, B_PIN = 10, G_PIN = 11;
// setup: pinMode ทั้ง 3 ขาเป็น OUTPUT แล้ว setRgb(0, 0, 0);

void setRgb(bool r, bool g, bool b) {    // Common Anode: LOW = ติด
  digitalWrite(R_PIN, r ? LOW : HIGH);
  digitalWrite(G_PIN, g ? LOW : HIGH);
  digitalWrite(B_PIN, b ? LOW : HIGH);
}
// เหลือง (1,1,0)  ฟ้า (0,1,1)  ม่วง (1,0,1)  ขาว (1,1,1)

// @@ show:seg | 7-Segment 1 หลัก (Common Cathode) | a–g → D2–D8
const int SEG_PINS[7] = {2, 3, 4, 5, 6, 7, 8};     // a b c d e f g
const byte DIGITS[10][7] = {
  {1,1,1,1,1,1,0}, {0,1,1,0,0,0,0}, {1,1,0,1,1,0,1}, {1,1,1,1,0,0,1}, {0,1,1,0,0,1,1},
  {1,0,1,1,0,1,1}, {1,0,1,1,1,1,1}, {1,1,1,0,0,0,0}, {1,1,1,1,1,1,1}, {1,1,1,1,0,1,1}
};
// setup: for (int i = 0; i < 7; i++) pinMode(SEG_PINS[i], OUTPUT);

void showDigit(int n) {                  // Common Anode ให้ใส่ !DIGITS[n][i]
  for (int i = 0; i < 7; i++) digitalWrite(SEG_PINS[i], DIGITS[n][i]);
}

// @@ show:matrix | LED Matrix บนบอร์ด R4 | ต้องติดตั้ง ArduinoGraphics
#include "ArduinoGraphics.h"
#include "Arduino_LED_Matrix.h"
ArduinoLEDMatrix matrix;
// setup: matrix.begin();

void showOnMatrix(String text) {         // ≤ 3 ตัวแสดงนิ่ง ยาวกว่านั้นเลื่อน
  matrix.beginDraw();
  matrix.clear();
  matrix.stroke(0xFFFFFFFF);
  matrix.textFont(Font_4x6);
  matrix.beginText(0, 1, 0xFFFFFF);
  matrix.print(text);
  if (text.length() <= 3) matrix.endText();
  else { matrix.textScrollSpeed(60); matrix.endText(SCROLL_LEFT); }
  matrix.endDraw();
}

// @@ mqtt:pub | Publish ค่าแบบต่าง ๆ | ข้อความใน MQTT เป็นตัวอักษรเสมอ
void publishExamples(int n, float t, float d) {
  mqtt.publish(TOPIC_PUB, String(n).c_str());          // จำนวนเต็ม  "512"
  mqtt.publish(TOPIC_PUB, String(t, 1).c_str());       // ทศนิยม 1 ตำแหน่ง "25.4"
  mqtt.publish(TOPIC_PUB, "ON");                       // ข้อความตรง ๆ
  mqtt.publish(TOPIC_PUB, String(n).c_str(), true, 0);    // retained: broker จำค่าล่าสุด

  // หลายค่าในข้อความเดียว (JSON) เช่น {"temp":25.4,"dist":12.0}
  String json = "{\"temp\":" + String(t, 1) + ",\"dist\":" + String(d, 1) + "}";
  mqtt.publish(TOPIC_PUB, json.c_str());
}

// @@ mqtt:sub | รับข้อความแล้วแปลงค่า | ใช้เป็น messageReceived ได้ทั้งฟังก์ชัน
void messageReceived(String &topic, String &payload) {
  String t = topic;
  String msg = payload;
  msg.trim();                            // ตัดช่องว่างหัวท้าย
  String up = msg;
  up.toUpperCase();                      // เทียบคำสั่งแบบไม่สนตัวเล็กใหญ่

  int   n = msg.toInt();                 // "128"  -> 128  (ไม่ใช่ตัวเลขได้ 0)
  float f = msg.toFloat();               // "25.4" -> 25.4

  if (t.endsWith("/led"))   setLed(n);   // subscribe หลาย topic แล้วแยกด้วยชื่อ topic
  if (t.endsWith("/temp"))  lcdLine(1, "T: " + String(f, 1));
  if (up == "ON")           setLed(255);
}
// setup: mqtt.onMessage(messageReceived);  และ subscribe ใน connectToMQTT()

// @@ mqtt:millis | ทำงานทุก X วินาทีโดยไม่ใช้ delay | ใช้ได้ทั้ง publish และไฟกระพริบ
unsigned long lastRun = 0;

void everySecond() {                     // เรียกใน loop() ทุกรอบ
  if (millis() - lastRun >= 1000) {      // 1000 ms = 1 วินาที
    lastRun = millis();
    // งานที่ต้องทำทุก 1 วินาที เช่น publish ค่าเซนเซอร์
  }
}

// @@ logic:max | หาค่ามากสุด / น้อยสุด | คืน index ใช้กับ array ชื่อได้เลย
int indexOfMax(float v[], int n) {
  int best = 0;
  for (int i = 1; i < n; i++) if (v[i] > v[best]) best = i;   // น้อยสุด: เปลี่ยน > เป็น <
  return best;
}
// ใช้: const char* NAME[3] = {"A","B","C"};  float v[3] = {...};
//      mqtt.publish(TOPIC_PUB, NAME[indexOfMax(v, 3)]);

// @@ logic:sort | เรียงลำดับหลายค่า | เรียงลำดับ index จากน้อยไปมาก ไม่ทำค่าเดิมเสีย
void sortIndex(float v[], int idx[], int n) {
  for (int i = 0; i < n; i++) idx[i] = i;
  for (int i = 0; i < n - 1; i++)
    for (int j = 0; j < n - 1 - i; j++)
      if (v[idx[j]] > v[idx[j + 1]]) { int t = idx[j]; idx[j] = idx[j + 1]; idx[j + 1] = t; }
}
// ใช้: int order[3]; sortIndex(v, order, 3);
//      NAME[order[0]] = น้อยสุด,  NAME[order[2]] = มากสุด,  NAME[order[1]] = ค่ากลาง (median)

// @@ logic:count | นับค่าที่เกินเกณฑ์ / ผลรวม / ค่าเฉลี่ย | เช่น มีกี่ห้องที่เกิน 50
int countAbove(float v[], int n, float limit) {
  int c = 0;
  for (int i = 0; i < n; i++) if (v[i] > limit) c++;
  return c;
}

float average(float v[], int n) {
  float sum = 0;
  for (int i = 0; i < n; i++) sum += v[i];
  return sum / n;
}
// แสดงเป็นจำนวนเต็มปัดเศษ: String((int)round(average(v, 3)))

// @@ logic:csv | แยกค่าหลายค่าในข้อความเดียว | "25.4,12,ON" -> field 0, 1, 2
String splitField(String s, char sep, int index) {
  int start = 0;
  for (int i = 0; i < index; i++) {
    start = s.indexOf(sep, start);
    if (start < 0) return "";            // ไม่มี field นั้น
    start++;
  }
  int end = s.indexOf(sep, start);
  if (end < 0) end = s.length();
  String out = s.substring(start, end);
  out.trim();
  return out;
}
// ใช้: float t = splitField(msg, ',', 0).toFloat();   int d = splitField(msg, ',', 1).toInt();
// ฝั่งส่ง:  String payload = String(t, 1) + "," + String(d);

// @@ logic:json | ดึงตัวเลขจาก JSON ง่าย ๆ | {"temp":25.4,"dist":12} ไม่ต้องลงไลบรารี
float jsonNumber(String json, String key) {
  int k = json.indexOf("\"" + key + "\"");
  if (k < 0) return NAN;                 // ไม่มี key นี้
  int colon = json.indexOf(':', k);
  if (colon < 0) return NAN;
  return json.substring(colon + 1).toFloat();   // toFloat หยุดเองเมื่อเจอ , หรือ }
}
// ใช้: float t = jsonNumber(msg, "temp");   if (!isnan(t)) { ... }

// @@ logic:hyst | เกณฑ์กันค่าแกว่ง (hysteresis) | ค่าอยู่ตรงเส้นพอดีจอจะไม่สลับไปมา
bool hot = false;

bool isHot(float t) {
  if (!hot && t > 30.5) hot = true;      // ขึ้นต้องเกิน 30.5
  if (hot && t < 29.5)  hot = false;     // ลงต้องต่ำกว่า 29.5
  return hot;                            // ระหว่าง 29.5–30.5 คงสถานะเดิม
}

// @@ logic:reverse | กลับหลังข้อความ | "REDRUM" -> "MURDER"
String reverseText(String s) {
  String out = "";
  for (int i = s.length() - 1; i >= 0; i--) out += s[i];
  return out;
}
// ตัวพิมพ์: s.toUpperCase();  s.toLowerCase();   เทียบ: if (s.equalsIgnoreCase("on"))

// @@hidden
void setup() {
  Serial.begin(9600);
  keypad.begin();
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  pinMode(LED_PIN, OUTPUT);
  lcd.init();
  lcd.backlight();
  i2cScan();
  matrix.begin();
  mqtt.onMessage(messageReceived);
}

void loop() {
  everySecond();
  String line = readKeypadLine();
  if (line.length() > 0) lcdLine(0, line);
  if (isPressed()) setRgb(1, 0, 0);
  showDigit(readPot() % 10);
  lcdLine(1, String(readDistanceCm(), 1) + " " + String(readMcp9700C(), 1) + " " + String(readThermistorC(), 1));
  showOnMatrix("12");
  publishExamples(1, 2.0, 3.0);

  float v[3] = {40, 15, 30};
  int order[3];
  sortIndex(v, order, 3);
  Serial.println(String(indexOfMax(v, 3)) + " " + String(order[0]) + " " + String(countAbove(v, 3, 20)) + " " + String(average(v, 3)));
  Serial.println(splitField("25.4,12,ON", ',', 2) + " " + String(jsonNumber("{\"temp\":25.4}", "temp")) + " " + String(isHot(31)) + " " + reverseText("REDRUM"));
}
