// งาน 12.2 : RGB LED แสดง 7 สี สีละ 1 วินาที
// บอร์ด Arduino UNO R4 WiFi
// RGB ที่แจกเป็นแบบ Common Anode (ตามโจทย์)
// วงจร: ขา Common (ขายาวสุด) -> 5V
//       R -> D9, B -> D10, G -> D11 (ผ่าน R 220Ω ทุกขา)

const int R_PIN = 9;
const int B_PIN = 10;
const int G_PIN = 11;

// Common Anode: ขาสีเป็น LOW = ติด, HIGH = ดับ (กลับด้านกับ LED ปกติ)
const bool COMMON_ANODE = true;

// 7 สี = ผสมแม่สีแบบเปิด/ปิด {R, G, B}
const int COLORS[7][3] = {
  {1, 0, 0},  // แดง
  {0, 1, 0},  // เขียว
  {0, 0, 1},  // น้ำเงิน
  {1, 1, 0},  // เหลือง  = แดง + เขียว
  {0, 1, 1},  // ฟ้า     = เขียว + น้ำเงิน
  {1, 0, 1},  // ม่วง    = แดง + น้ำเงิน
  {1, 1, 1}   // ขาว     = ทุกสี
};
const char* NAMES[7] = {"Red", "Green", "Blue", "Yellow", "Cyan", "Magenta", "White"};

void setColor(int r, int g, int b) {
  if (COMMON_ANODE) { r = !r; g = !g; b = !b; }
  digitalWrite(R_PIN, r);
  digitalWrite(G_PIN, g);
  digitalWrite(B_PIN, b);
}

void setup() {
  Serial.begin(9600);
  pinMode(R_PIN, OUTPUT);
  pinMode(G_PIN, OUTPUT);
  pinMode(B_PIN, OUTPUT);
}

void loop() {
  for (int i = 0; i < 7; i++) {
    setColor(COLORS[i][0], COLORS[i][1], COLORS[i][2]);
    Serial.println(NAMES[i]);
    delay(1000);
  }
}
