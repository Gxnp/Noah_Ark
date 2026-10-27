วิธีใช้
=======
1. แตก zip ก่อน (อย่าเปิดไฟล์จากในตัว zip ตรงๆ)
2. ดับเบิลคลิกไฟล์ .ino ในโฟลเดอร์ที่ต้องการ -> Arduino IDE จะเปิดให้เลย ไม่ต้อง copy-paste
   (ชื่อโฟลเดอร์ต้องตรงกับชื่อไฟล์ .ino ห้ามเปลี่ยนแค่อย่างใดอย่างหนึ่ง)
3. งาน MQTT: แก้ WIFI_SSID / WIFI_PASSWORD / Topic / MQTT_CLIENT_ID ก่อนอัปโหลด
   MQTT_CLIENT_ID ต้องไม่ซ้ำกับคนอื่นในห้อง ไม่งั้นจะหลุดกันไปมา

Library ที่ต้องติดตั้ง (Tools > Manage Libraries)
- MQTTClient  (โดย Joel Gaehwiler)        -> L06a, L06b, L08, L09, T1
- Adafruit Keypad                          -> L07, L08
- LiquidCrystal I2C (โดย Frank de Brabander) -> T3
- WiFiS3 มากับบอร์ด Arduino UNO R4 WiFi อยู่แล้ว

ถ้าเจอ error "stray '\302' in program" อีก
- เกิดจากตัวเว้นวรรคพิเศษ (non-breaking space) ที่ติดมาตอน copy จากเว็บ/PDF/Notion
- วิธีแก้เร็ว: ใน Arduino IDE ลบช่องว่างหน้าบรรทัดที่ error แล้วกด Tab ใหม่
  หรือกด Ctrl+T (Auto Format) หลังลบ หรือเปิดไฟล์ .ino จากโฟลเดอร์นี้แทนการ copy

สิ่งที่แก้จากต้นฉบับ
- ลบ non-breaking space ทั้งหมด 347 ตัว
- L03: เพิ่ม delay(1000) ไม่งั้นตัวเลขเปลี่ยนเร็วจนมองไม่เห็น
- L06a: เติม ; หลัง analogWrite(LED_PIN, data) และเพิ่ม pinMode
- L07: เติม } ที่ขาดไป 2 ตัว
