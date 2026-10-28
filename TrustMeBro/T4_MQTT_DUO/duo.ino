#include <WiFiS3.h>
#include <MQTTClient.h>

int LED_PIN = 6;

const char WIFI_SSID[] = "Napon";    // CHANGE TO YOUR WIFI SSID
const char WIFI_PASSWORD[] = "11501112";  // CHANGE TO YOUR WIFI PASSWORD
const char MQTT_BROKER_ADRRESS[] = "phycom.it.kmitl.ac.th";  // CHANGE TO MQTT BROKER'S ADDRESS
const int MQTT_PORT = 1883;
const char MQTT_CLIENT_ID[] = "napon_ard";  // CHANGE IT AS YOU DESIRE
const char MQTT_USERNAME[] = "";            // CHANGE IT IF REQUIRED
const char MQTT_PASSWORD[] = "";            // CHANGE IT IF REQUIRED

// Topic สำหรับการรับค่าเพื่อนำมาบวกกัน
const char TOPIC_A[] = "68070077/venus";
const char TOPIC_B[] = "68070077/prob_stat";

const char PUBLISH_TOPIC[] = "68070077/temp";

// 🔴 แก้ไขจุดที่ 1: เปลี่ยนระยะเวลาส่งข้อมูลเป็น 3000 ms (3 วินาที)
const int PUBLISH_INTERVAL = 3000;  

// ตัวแปรสำหรับเก็บค่าตัวเลขล่าสุดจากทั้ง 2 topic
int valA = 0;
int valB = 0;

WiFiClient network;
MQTTClient mqtt = MQTTClient(256);

unsigned long lastPublishTime = 0;

void setup() {
  Serial.begin(9600);
  pinMode(LED_PIN, OUTPUT);
  
  int status = WL_IDLE_STATUS;
  while (status != WL_CONNECTED) {
    Serial.print("Arduino UNO R4 - Attempting to connect to SSID: ");
    Serial.println(WIFI_SSID);
    status = WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    delay(10000);
  }
  
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());
  connectToMQTT();
}

void loop() {
  mqtt.loop();

  // 🔴 แก้ไขจุดที่ 3: เช็คการเชื่อมต่อ MQTT หากหลุดให้ต่อใหม่ทันที
  if (!mqtt.connected()) {
    connectToMQTT();
  }

  if (millis() - lastPublishTime > PUBLISH_INTERVAL) {
    sendToMQTT();
    lastPublishTime = millis();
  }
}

void connectToMQTT() {
  mqtt.begin(MQTT_BROKER_ADRRESS, MQTT_PORT, network);
  mqtt.onMessage(messageReceived);
  Serial.print("Arduino UNO R4 - Connecting to MQTT broker");
  
  while (!mqtt.connect(MQTT_CLIENT_ID, MQTT_USERNAME, MQTT_PASSWORD)) {
    Serial.print(".");
    delay(500);
  }
  Serial.println();
  
  if (!mqtt.connected()) {
    Serial.println("Arduino UNO R4 - MQTT broker Timeout!");
    return;
  }
  
  // Subscribe ทั้ง 2 topics
  subscribeTopic(TOPIC_A);
  subscribeTopic(TOPIC_B);
  
  Serial.println("Arduino UNO R4 - MQTT broker Connected!");
}

void subscribeTopic(const char *topic) {
  if (mqtt.subscribe(topic)) {
    Serial.print("Arduino UNO R4 - Subscribed to the topic: ");
  } else {
    Serial.print("Arduino UNO R4 - Failed to subscribe to the topic: ");
  }
  Serial.println(topic);
}

// 🔴 แก้ไขจุดที่ 2: เขียนฟังก์ชันส่งผลรวมกลับไปยัง MQTT Server
void sendToMQTT() {
  int sum = valA + valB;
  String sumStr = String(sum);

  mqtt.publish(PUBLISH_TOPIC, sumStr);

  Serial.println("Arduino UNO R4 - Sent to MQTT:");
  Serial.print("- topic: ");
  Serial.println(PUBLISH_TOPIC);
  Serial.print("- payload (Sum): ");
  Serial.println(sumStr);
  Serial.println("---------------------------");
}

void messageReceived(String &topic, String &payload) {
  Serial.println("Arduino UNO R4 - received from MQTT:");
  Serial.println("- topic: " + topic);
  Serial.println("- payload: " + payload);

  // เช็คว่าข้อความมาจาก Topic ไหนแล้วแปลงค่าเป็น integer
  if (topic == TOPIC_A) {
    valA = payload.toInt();
  } else if (topic == TOPIC_B) {
    valB = payload.toInt();
  }

  // แสดงค่าล่าสุดบน Serial Monitor
  int sum = valA + valB;
  Serial.print("Current Values -> A: ");
  Serial.print(valA);
  Serial.print(" | B: ");
  Serial.print(valB);
  Serial.print(" | Sum: ");
  Serial.println(sum);
  Serial.println("---------------------------");
}