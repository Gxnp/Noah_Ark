void setup() {
  pinMode(5,INPUT_PULLUP);
  Serial.begin(9600);
}

void loop() {
  int data = digitalRead(5);
  Serial.println(data);
  delay(100);
}
