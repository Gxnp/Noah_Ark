// งาน 15.2 : MQTT Keypad — ฝั่งคนรับ (Subscriber) แสดงบนจอ LCD
// subscribe ค่าปุ่มจากบอร์ดเพื่อน (W15_2_MQTT_Keypad_Sender) แล้วแสดงบน LCD 16x2 I2C
// กด * ที่ฝั่งส่ง = ล้างจอ
// บอร์ด Arduino UNO R4 WiFi   ไลบรารี: MQTT (MQTTClient.h), LiquidCrystal_I2C (Frank de Brabander)
// วงจร LCD I2C: GND -> GND, VCC -> 5V, SDA -> SDA (A4), SCL -> SCL (A5)

#include <WiFiS3.h>
#include <MQTTClient.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

// ===== แก้ตรงนี้ (topic ต้องเหมือนกับฝั่งคนส่ง) =====
const char WIFI_SSID[] = "iphone 80 promax";
const char WIFI_PASSWORD[] = "88888888";
const char MQTT_BROKER_ADRRESS[] = "mqtt-dashboard.com";
const int   MQTT_PORT = 1883;
const char* TOPIC_SUB = "68070180/keypad";

LiquidCrystal_I2C lcd(0x27, 16, 2);   // ถ้าจอไม่ขึ้น ลอง 0x3F
WiFiClient network;
MQTTClient mqtt = MQTTClient(256);
String text = "";                     // ข้อความที่รับมาสะสม

// พิมพ์ข้อความเต็มบรรทัด เติมช่องว่างทับของเก่า จอไม่กระพริบ
void lcdLine(int row, String s) {
  lcd.setCursor(0, row);
  while (s.length() < 16) s += ' ';
  lcd.print(s.substring(0, 16));
}

void messageReceived(String &topic, String &payload) {
  String msg = payload;
  msg.trim();
  (void)topic;
  Serial.print("[RX] ");
  Serial.println(msg);

  if (msg == "*") text = "";                       // ล้างข้อความ
  else            text += msg;
  if (text.length() > 16) text = text.substring(text.length() - 16);   // เก็บแค่ 16 ตัวท้าย

  lcdLine(0, "Key: " + msg);
  lcdLine(1, text);
}

void connectWiFi() {
  lcdLine(0, "WiFi...");
  while (WiFi.begin(WIFI_SSID, WIFI_PASSWORD) != WL_CONNECTED) {
    Serial.print(".");
    delay(3000);
  }
  while (WiFi.localIP() == IPAddress(0, 0, 0, 0)) delay(200);
  Serial.print("WiFi OK, IP = ");
  Serial.println(WiFi.localIP());
}

void connectToMQTT() {
  while (!mqtt.connected()) {
    lcdLine(0, "MQTT...");
    String clientId = "r4-lcd-" + String(random(0xFFFFFF), HEX);
    if (mqtt.connect(clientId.c_str())) {
      mqtt.subscribe(TOPIC_SUB);
      Serial.println("MQTT OK");
      lcdLine(0, "Waiting key...");
    } else {
      Serial.print("MQTT fail err=");
      Serial.print(mqtt.lastError()); Serial.print(" rc="); Serial.println(mqtt.returnCode());
      delay(2000);
    }
  }
}

void setup() {
  Serial.begin(9600);
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
}
