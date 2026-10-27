#include <WiFiS3.h>
#include <MQTTClient.h>

int LED_PIN = 6;
const char WIFI_SSID[] = "WIFI_SSID";    // CHANGE TO YOUR WIFI SSID
const char WIFI_PASSWORD[] = "WIFI_PASSWORD";  // CHANGE TO YOUR WIFI PASSWORD
const char MQTT_BROKER_ADRRESS[] = "mqtt-dashboard.com";  // CHANGE TO MQTT BROKER'S ADDRESS
//const char MQTT_BROKER_ADRRESS[] = "192.168.0.11";  // CHANGE TO MQTT BROKER'S IP ADDRESS
const int MQTT_PORT = 1883;
const char MQTT_CLIENT_ID[] = "yossawat_arduino";  // CHANGE IT AS YOU DESIRE
const char MQTT_USERNAME[] = "";                   // CHANGE IT IF REQUIRED, empty if not required
const char MQTT_PASSWORD[] = "";                   // CHANGE IT IF REQUIRED, empty if not required
// The MQTT topics that Arduino should publish/subscribe
const char PUBLISH_TOPIC[] = "m/led";       // CHANGE IT AS YOU DESIRE
const char SUBSCRIBE_TOPIC[] = "yossawat_arduino/receive";  // CHANGE IT AS YOU DESIRE
const char TEMP_TOPIC[] = "yossawat_arduino/temperature";
const char LED_TOPIC[] = "yossawat_arduino/led";
const int PUBLISH_INTERVAL = 100;  // 5 seconds

WiFiClient network;
MQTTClient mqtt = MQTTClient(256);

unsigned long lastPublishTime = 0;

void setup() {
  Serial.begin(9600);
  int status = WL_IDLE_STATUS;
  while (status != WL_CONNECTED) {
    Serial.print("Arduino UNO R4 - Attempting to connect to SSID: ");
    Serial.println(WIFI_SSID);
    // Connect to WPA/WPA2 network. Change this line if using open or WEP network:
    status = WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    // wait 10 seconds for connection:
    delay(10000);
  }
  // print your board's IP address:
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());
  connectToMQTT();
}

void loop() {
  mqtt.loop();
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
    delay(100);
  }
  Serial.println();
  if (!mqtt.connected()) {
    Serial.println("Arduino UNO R4 - MQTT broker Timeout!");
    return;
  }
  subscribeTopic(SUBSCRIBE_TOPIC);
  subscribeTopic(LED_TOPIC);
  subscribeTopic(TEMP_TOPIC);
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



void sendToMQTT() {
  // int val = millis() / 1000;
  int val = analogRead(A0);
  String val_str = String(val);
  char messageBuffer[10];
  val_str.toCharArray(messageBuffer, 10);
  mqtt.publish(PUBLISH_TOPIC, messageBuffer);
  Serial.println("Arduino UNO R4 - sent to MQTT:");
  Serial.print("- topic: ");
  Serial.println(PUBLISH_TOPIC);
  Serial.print("- payload:");
  Serial.println(messageBuffer);
}



void messageReceived(String &topic, String &payload) {
  Serial.println("Arduino UNO R4 - received from MQTT:");
  Serial.println("- topic: " + topic);
  Serial.println("- payload:");
}
