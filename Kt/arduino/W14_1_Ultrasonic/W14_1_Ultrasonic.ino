// งาน 14.1 : Ultrasonic Sensor (HC-SR04) วัดระยะทาง แสดงผลทาง Serial Monitor
// บอร์ด Arduino UNO R4 WiFi
// วงจร: VCC -> 5V, Trig -> D4, Echo -> D2, GND -> GND (ตามสไลด์)

const int TRIG_PIN = 4;
const int ECHO_PIN = 2;

void setup() {
  Serial.begin(9600);
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
}

void loop() {
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);          // ยิงคลื่นเสียง 10 µs
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);

  long pulseWidth = pulseIn(ECHO_PIN, HIGH, 30000);   // เวลาไป-กลับ (µs), รอสูงสุด 30 ms

  Serial.print("Pulse Width: ");
  Serial.println(pulseWidth);

  if (pulseWidth == 0) {
    Serial.println("Distance: out of range");
  } else {
    // เสียงเดิน 1 cm ใช้ประมาณ 29 µs และต้องหาร 2 เพราะไป-กลับ
    long distance = pulseWidth / 29 / 2;
    Serial.print("Distance: ");
    Serial.print(distance);
    Serial.println(" cm");
  }
  delay(1000);
}
