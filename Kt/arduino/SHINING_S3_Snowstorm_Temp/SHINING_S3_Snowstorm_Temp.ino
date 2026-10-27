// =====================================================================
// แนวข้อสอบ The Shining  S3 : พายุหิมะ / เขาวงกตต้นไม้ที่หนาวจนแข็ง (เดา · แบบ Final 68 ข้อ 3)
// อ่านเซ็นเซอร์อุณหภูมิ -> publish ค่า °C ไปที่ student_id/temp ทุก 1 วินาที
// และ publish สถานะไปที่ student_id/weather
//   ต่ำกว่า COLD_C  -> "Frozen"   (Jack แข็งในเขาวงกต)
//   ตั้งแต่ COLD_C   -> "Snowing"
// LCD (ถ้าต่อ): บรรทัด 1 "Overlook Temp:" บรรทัด 2 "27.4 C Snowing"
//
// บอร์ด Arduino UNO R4 WiFi   ไลบรารี: MQTT (MQTTClient.h), LiquidCrystal_I2C
// วงจร MCP9700: VDD -> 5V, Vout -> A1, GND -> GND (ด้านแบนหันเข้าหาตัว)
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
const String TOPIC_TEMP_NAME  = "/temp";              // <<< เปลี่ยนตามโจทย์จริง
const String TOPIC_STATE_NAME = "/weather";
const float  COLD_C = 25.0;                           // เกณฑ์ (ห้องแอร์ ~24–27 °C ลองเป่าลม/จับเซ็นเซอร์ทดสอบ)
const int    SENSOR_TYPE = 1;                         // 1 = MCP9700/TMP36, 2 = LM35

const int TEMP_PIN = A1;

LiquidCrystal_I2C lcd(0x27, 16, 2);
WiFiClient network;
MQTTClient mqtt = MQTTClient(256);
String T_TEMP, T_STATE;
unsigned long lastPub = 0;

void lcdLine(int row, String s) {
  lcd.setCursor(0, row);
  while (s.length() < 16) s += ' ';
  lcd.print(s.substring(0, 16));
}

float readTempC() {
  long sum = 0;
  for (int i = 0; i < 10; i++) { sum += analogRead(TEMP_PIN); delay(2); }
  float v = (sum / 10.0) * 5.0 / 1023.0;
  return SENSOR_TYPE == 2 ? v * 100.0 : (v - 0.5) / 0.01;
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
    if (mqtt.connect(id.c_str())) { Serial.println("MQTT OK"); lcdLine(0, "Overlook Temp:"); }
    else { Serial.print("MQTT fail err="); Serial.print(mqtt.lastError()); Serial.print(" rc="); Serial.println(mqtt.returnCode()); delay(2000); }
  }
}

void setup() {
  Serial.begin(9600);
  lcd.init();
  lcd.backlight();
  T_TEMP  = STUDENT_ID + TOPIC_TEMP_NAME;
  T_STATE = STUDENT_ID + TOPIC_STATE_NAME;
  connectWiFi();
  randomSeed(micros());
  mqtt.begin(MQTT_BROKER_ADRRESS, MQTT_PORT, network);
  connectToMQTT();
}

void loop() {
  if (WiFi.status() != WL_CONNECTED) connectWiFi();
  if (!mqtt.connected()) connectToMQTT();
  mqtt.loop();

  if (millis() - lastPub >= 1000) {
    lastPub = millis();
    float t = readTempC();
    String state = (t < COLD_C) ? "Frozen" : "Snowing";
    mqtt.publish(T_TEMP.c_str(), String(t, 1).c_str());
    mqtt.publish(T_STATE.c_str(), state.c_str());
    lcdLine(0, "Overlook Temp:");
    lcdLine(1, String(t, 1) + (char)223 + "C " + state);
    Serial.println("[TX] " + T_TEMP + " : " + String(t, 1) + "  " + T_STATE + " : " + state);
  }
}
