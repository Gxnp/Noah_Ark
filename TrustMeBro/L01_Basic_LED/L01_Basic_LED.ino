int LED_PIN = 6;

void setup() {
  pinMode(LED_PIN, OUTPUT);
}

void loop() {
  for (int i = 0; i < 255; i++) {
    analogWrite(LED_PIN, i);
    delay(10);
  }
}
