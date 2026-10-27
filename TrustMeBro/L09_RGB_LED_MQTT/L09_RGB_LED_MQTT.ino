#include <WiFiS3.h>
#include <MQTTClient.h>

int RED_PIN = 2;
int GREEN_PIN = 3;
int BLUE_PIN = 4;

const char WIFI_SSID[] = "WIFI_SSID";
const char WIFI_PASSWORD[] = "WIFI_PASSWORD";

const char MQTT_BROKER_ADRRESS[] = "mqtt-dashboard.com";
const int MQTT_PORT = 1883;

const char MQTT_CLIENT_ID[] = "yossawat_arduino";
const char MQTT_USERNAME[] = "";
const char MQTT_PASSWORD[] = "";

const char PUBLISH_TOPIC[] = "yossawat_arduino/send";
const char SUBSCRIBE_TOPIC[] = "meen_arduino/receive";
const char TEMP_TOPIC[] = "yossawat_arduino/temperature";
const char LED_TOPIC[] = "yossawat_arduino/led";

const int PUBLISH_INTERVAL = 5000;

WiFiClient network;
MQTTClient mqtt = MQTTClient(256);

unsigned long lastPublishTime = 0;

void setup() {
  Serial.begin(9600);

  pinMode(RED_PIN, OUTPUT);
  pinMode(GREEN_PIN, OUTPUT);
  pinMode(BLUE_PIN, OUTPUT);

  digitalWrite(RED_PIN, !LOW);
  digitalWrite(GREEN_PIN, !LOW);
  digitalWrite(BLUE_PIN, !LOW);

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
  int val = millis() / 1000;
  // int val = analogRead(A0);

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
  Serial.println(payload);

  payload.trim();

  if (payload == "r-on") {
    digitalWrite(RED_PIN, !HIGH);
    Serial.println("r-onnn");
  } else if (payload == "r-off") {
    digitalWrite(RED_PIN, !LOW);
  } else if (payload == "g-on") {
    digitalWrite(GREEN_PIN, !HIGH);
  } else if (payload == "g-off") {
    digitalWrite(GREEN_PIN, !LOW);
  } else if (payload == "b-on") {
    digitalWrite(BLUE_PIN, !HIGH);
  } else if (payload == "b-off") {
    digitalWrite(BLUE_PIN, !LOW);
  }
}
