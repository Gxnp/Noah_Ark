// =====================================================================
// สไลด์ PC69-15 "การทดลองอื่นๆ" : อุณหภูมิ -> MQTT -> LCD + RGB
//   คนส่ง (Publisher): อ่าน MCP9700 แล้ว publish อุณหภูมิทุก 1 วินาที
//   คนรับ (Subscriber): แสดงอุณหภูมิบน LCD (ทศนิยม 2 ตำแหน่ง)
//                       Color LED ร้อน = แดง  เย็น = เขียว (เกณฑ์ HOT_C)
// ไฟล์นี้ทำได้ทั้ง 2 บทบาทในบอร์ดเดียว (ทำคนเดียว ตั้ง PUBLISH_TOPIC = SUBSCRIBE_TOPIC)
//
// บอร์ด Arduino UNO R4 WiFi   ไลบรารี: MQTT (MQTTClient.h), LiquidCrystal_I2C
// วงจร MCP9700: VDD -> 5V, Vout -> A1, GND -> GND (ด้านแบนหันเข้าหาตัว)
//      LCD I2C: SDA -> A4 (SDA), SCL -> A5 (SCL), VCC -> 5V, GND -> GND
//      Color LED (Common Anode): Common -> 5V, R -> D9, B -> D10, G -> D11 (R 220–330Ω)
// =====================================================================

#include <WiFiS3.h>
#include <MQTTClient.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

const char WIFI_SSID[] = "iphone 80 promax";
const char WIFI_PASSWORD[] = "88888888";

const char MQTT_BROKER_ADRRESS[] = "mqtt-dashboard.com";  // หรือ "phycom.it.kmitl.ac.th"
const int MQTT_PORT = 1883;
const char MQTT_USERNAME[] = "";
const char MQTT_PASSWORD[] = "";

const char PUBLISH_TOPIC[]   = "68070180/temperature";
const char SUBSCRIBE_TOPIC[] = "68070180/temperature";

const float HOT_C = 30.0;          // มากกว่านี้ = ร้อน (แดง)  ไม่ถึง = เย็น (เขียว)
const int TEMP_PIN = A1;
const int R_PIN = 9, B_PIN = 10, G_PIN = 11;
const int PUBLISH_INTERVAL = 1000;

LiquidCrystal_I2C lcd(0x27, 16, 2);
WiFiClient network;
MQTTClient mqtt = MQTTClient(256);
unsigned long lastPublishTime = 0;

byte degreeChar[8] = {0b00110, 0b01001, 0b01001, 0b00110, 0b00000, 0b00000, 0b00000, 0b00000};   // °

void setRgb(bool r, bool g, bool b) {                  // Common Anode: LOW = ติด
  digitalWrite(R_PIN, r ? LOW : HIGH);
  digitalWrite(G_PIN, g ? LOW : HIGH);
  digitalWrite(B_PIN, b ? LOW : HIGH);
}

float readTempC() {
  // สูตรในสไลด์: Temp = 100 * (V - 1) + 50   (เท่ากับ (V - 0.5) / 0.01)
  long sum = 0;
  for (int i = 0; i < 10; i++) { sum += analogRead(TEMP_PIN); delay(2); }
  float v = (sum / 10.0) * 5.0 / 1023.0;
  return 100.0 * (v - 1.0) + 50.0;
}

void messageReceived(String &topic, String &payload) {
  Serial.println("- received " + topic + " : " + payload);
  float t = payload.toFloat();

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Temperature:");
  lcd.setCursor(0, 1);
  lcd.print(t, 2);                                     // ทศนิยม 2 ตำแหน่ง
  lcd.write(byte(0));                                  // °
  lcd.print("C ");
  lcd.print(t > HOT_C ? "HOT" : "COLD");

  if (t > HOT_C) setRgb(1, 0, 0);                      // ร้อน : แดง
  else           setRgb(0, 1, 0);                      // เย็น : เขียว
}

void connectToMQTT() {
  mqtt.begin(MQTT_BROKER_ADRRESS, MQTT_PORT, network);
  mqtt.onMessage(messageReceived);
  Serial.print("Arduino UNO R4 - Connecting to MQTT broker");
  String clientId = "68070180-temp-" + String(random(0xFFFF), HEX);
  while (!mqtt.connect(clientId.c_str(), MQTT_USERNAME, MQTT_PASSWORD)) {
    Serial.print(".");
    delay(100);
  }
  Serial.println();
  mqtt.subscribe(SUBSCRIBE_TOPIC);
  Serial.println("Arduino UNO R4 - MQTT broker Connected!");
}

void setup() {
  Serial.begin(9600);
  pinMode(R_PIN, OUTPUT); pinMode(G_PIN, OUTPUT); pinMode(B_PIN, OUTPUT);
  setRgb(0, 0, 0);
  lcd.init();
  lcd.backlight();
  lcd.createChar(0, degreeChar);
  lcd.print("WiFi...");

  int status = WL_IDLE_STATUS;
  while (status != WL_CONNECTED) {
    Serial.print("Arduino UNO R4 - Attempting to connect to SSID: ");
    Serial.println(WIFI_SSID);
    status = WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    delay(5000);
  }
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());
  randomSeed(micros());
  connectToMQTT();
  lcd.clear();
  lcd.print("Waiting temp...");
}

void loop() {
  mqtt.loop();
  if (!mqtt.connected()) connectToMQTT();

  if (millis() - lastPublishTime > PUBLISH_INTERVAL) {
    float t = readTempC();
    String val_str = String(t, 2);
    char messageBuffer[10];
    val_str.toCharArray(messageBuffer, 10);            // แบบเดียวกับสไลด์
    mqtt.publish(PUBLISH_TOPIC, messageBuffer);
    Serial.print("- sent ");
    Serial.println(messageBuffer);
    lastPublishTime = millis();
  }
}
