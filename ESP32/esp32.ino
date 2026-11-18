#include <WiFi.h>
#include <WiFiManager.h>
#include <WebSocketsServer.h>

// ==========================================
// ตั้งค่าขาปุ่มกด (GPIO)
// ==========================================
const int BTN_LEFT_PIN  = 22; // ปุ่มซ้าย (L) ต่อระหว่าง GPIO 22 กับ GND
const int BTN_RIGHT_PIN = 23; // ปุ่มขวา (R) ต่อระหว่าง GPIO 23 กับ GND

WebSocketsServer webSocket = WebSocketsServer(81);

bool lastLeftState  = HIGH;
bool lastRightState = HIGH;
unsigned long lastLeftTime  = 0;
unsigned long lastRightTime = 0;
const unsigned long DEBOUNCE_DELAY = 25;

void onWebSocketEvent(uint8_t client_num, WStype_t type, uint8_t * payload, size_t length) {
  switch (type) {
    case WStype_DISCONNECTED:
      Serial.printf("[WS] เครื่องที่ %u ตัดการเชื่อมต่อ\n", client_num);
      break;
    case WStype_CONNECTED: {
      IPAddress ip = webSocket.remoteIP(client_num);
      Serial.printf("[WS] เครื่องที่ %u เชื่อมต่อสำเร็จจาก IP: %s\n", client_num, ip.toString().c_str());
      break;
    }
  }
}

void setup() {
  Serial.begin(115200);
  delay(200);

  pinMode(BTN_LEFT_PIN, INPUT_PULLUP);
  pinMode(BTN_RIGHT_PIN, INPUT_PULLUP);

  // เริ่มต้นระบบ WiFiManager
  WiFiManager wm;

  // [ข้อสังเกต] ถ้าต้องการล้างค่า Wi-Fi เก่าทั้งหมดเพื่อทดสอบหน้าเว็บตั้งค่าใหม่ 
  // ให้ปลดคอมเมนต์ (ลบเครื่องหมาย //) บรรทัดด้านล่างนี้แค่อัปโหลดรอบแรก แล้วค่อยใส่กลับทีหลัง
  // wm.resetSettings();

  // กำหนดชื่อ Wi-Fi ที่บอร์ดจะปล่อยออกมาให้เราเข้าไปตั้งค่า (เช่น ตอนอยู่นอกบ้านแล้วไม่มีเน็ตจำ)
  // ชื่อ Wi-Fi: RhythmGame_Setup | รหัสผ่าน: 12345678 (ตั้งค่าว่างไว้ได้ถ้าไม่อยากใส่รหัส)
  bool res = wm.autoConnect("RhythmGame_Setup", "12345678");

  if (!res) {
    Serial.println("เชื่อมต่อไม่สำเร็จและหมดเวลาตั้งค่า");
    // ESP.restart();
  } else {
    Serial.println("เชื่อมต่อ Wi-Fi สำเร็จแล้ว!");
  }

  Serial.print("IP Address สำหรับใช้งาน: http://");
  Serial.println(WiFi.localIP());

  // เริ่มต้น WebSocket Server พอร์ต 81
  webSocket.begin();
  webSocket.onEvent(onWebSocketEvent);
}

void loop() {
  webSocket.loop();

  unsigned long currentMillis = millis();

  // --- ตรวจสอบปุ่มซ้าย (L) ---
  bool currentLeftState = digitalRead(BTN_LEFT_PIN);
  if (currentLeftState != lastLeftState) {
    if (currentMillis - lastLeftTime > DEBOUNCE_DELAY) {
      if (currentLeftState == LOW) {
        webSocket.broadcastTXT("L");
        Serial.println("ส่งคำสั่ง: L");
      } else {
        webSocket.broadcastTXT("L_UP");
      }
      lastLeftTime = currentMillis;
      lastLeftState = currentLeftState;
    }
  }

  // --- ตรวจสอบปุ่มขวา (R) ---
  bool currentRightState = digitalRead(BTN_RIGHT_PIN);
  if (currentRightState != lastRightState) {
    if (currentMillis - lastRightTime > DEBOUNCE_DELAY) {
      if (currentRightState == LOW) {
        webSocket.broadcastTXT("R");
        Serial.println("ส่งคำสั่ง: R");
      } else {
        webSocket.broadcastTXT("R_UP");
      }
      lastRightTime = currentMillis;
      lastRightState = currentRightState;
    }
  }
}