# 01

```cpp
void handleSub2(String msg) {
  msg.trim();
  if (msg.length() == 0 || !isDigit(msg[0])) return;   // ไม่ใช่ตัวเลข ไม่ทำอะไร
  int n = msg.toInt();
  if (n < 0 || n > 9) return;

  showDigit(n);                                  // 7-Segment แสดงเลข

  // ===== LED: แก้เงื่อนไขให้ตรงโจทย์ =====
  if (n <= 3)      setColor(false, true, false); // 0-3 เขียว
  else if (n <= 6) setColor(false, false, true); // 4-6 ฟ้า
  else             setColor(true, false, false); // 7-9 แดง
}
```
ไม่ต้องเขียนไฟล์ใหม่ครับ ใช้ไฟล์ EXAM_2PUB_2SUB.ino แล้วแทนฟังก์ชัน handleSub2() ทั้งก้อนด้วยโค้ดนี้ พอได้รับเลขมา 7-seg จะแสดงเลข และ LED จะติดไปพร้อมกัน:

ถ้าโจทย์กำหนดเงื่อนไขอื่น ให้แก้แค่ส่วน if ด้านล่าง เช่น
เลขคู่/คี่: if (n % 2 == 0) setColor(false, true, false); else setColor(true, false, false);
ใช้ LED ธรรมดา 1 ดวงแทน Color LED: ต่อที่ A3 เพราะ D2–D8 ใช้กับ 7-seg และ D13 ใช้กับ Ultrasonic อยู่แล้ว
ใน setup() เพิ่ม pinMode(A3, OUTPUT);
ส่วนเงื่อนไขเขียนเป็น digitalWrite(A3, n >= 5 ? HIGH : LOW); (ตัวอย่าง: ติดเมื่อเลข ≥ 5)

ต่อสายเหมือนเดิม: 7-seg a–g → D2–D8 และ COM → GND ส่วน Color LED ต่อ R → D9, B → D10, G → D11 และขายาว → 5V


# 02

```cpp
String getPub1() {
  int raw = analogRead(POT_PIN);                 // 0 - 1023
  int percent = map(raw, 0, 1023, 0, 100);       // 0 - 100
  return String(percent);                        // เช่น "57"
}
```

ใช้ map() แปลงค่าครับ ใน EXAM_2PUB_2SUB.ino ให้แทน getPub1() ด้วยโค้ดนี้:

ค่าจริงของ analogRead คือ 0–1023 ไม่ใช่ 1024 ถ้าเขียนเป็น 1024 พอหมุนสุดจะได้แค่ 99 ไม่ถึง 100

ถ้าโจทย์อยากได้ทศนิยม ใช้แบบนี้แทน:
  float percent = raw * 100.0 / 1023.0;
  return String(percent, 1);                     // เช่น "57.3"


ถ้าต้องมีเครื่องหมาย % ต่อท้ายด้วย เปลี่ยนบรรทัด return เป็น return String(percent) + "%";

ต่อ Pot แบบเดิม: ขาริมข้างหนึ่งไป 5V ขากลางไป A0 ขาริมอีกข้างไป GND