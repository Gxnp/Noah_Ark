void setup() {
  Serial.begin(9600);
  pinMode(11, INPUT);
  pinMode(10, OUTPUT);
}

void loop() {
  digitalWrite(10, HIGH);
  delayMicroseconds(10);
  digitalWrite(10, LOW);

  int pulseWidth = pulseIn(11, HIGH);

  Serial.print("Pulse width: ");
  Serial.println(pulseWidth);
  long distance = pulseWidth / 58;
  Serial.print("Distance: ");
  Serial.println(distance);
  delay(1000);
}
