/*
 * ESP32 Controller for 4-Servo Robotic Arm via Wi-Fi & WebSockets (PCA9685 I2C Driver)
 * ------------------------------------------------------------------------------------
 * Project: SIH2026 Integrated Robot Controller
 * Component: Robotic Arm (Independent Wi-Fi WebSocket Client)
 * Location: hardware/robot-test/arm-slave/arm-slave.ino
 *
 * System Role:
 *   - Connects directly to local Wi-Fi.
 *   - Maintains persistent WebSocket client connection to FastAPI server:
 *       /api/v1/arm/ws/device  --> 4 Servo angles (servo1, servo2, servo3, servo4)
 *   - Drives 4 RC Servos via Adafruit PCA9685 PWM Driver over I2C (SDA=21, SCL=22).
 *   - Operates independently from Robot Car ESP32 over Wi-Fi (No SPI required).
 *
 * PCA9685 I2C Pin Assignments:
 *   - SDA : GPIO 21
 *   - SCL : GPIO 22
 *   - Channels: 0 -> Servo 1 (Base), 1 -> Servo 2 (Shoulder), 2 -> Servo 3 (Wrist), 3 -> Servo 4 (Gripper)
 *
 * Board in Arduino IDE: "ESP32 Dev Module"
 * Required Libraries:
 *   - Adafruit PWM Servo Driver Library
 *   - ArduinoJson by Benoit Blanchon (Version 6 or 7)
 *   - Wire (Built-in ESP32)
 *   - WiFi (Built-in ESP32)
 */

#include <Wire.h>
#include <Adafruit_PWMServoDriver.h>
#include <WiFi.h>
#include <ArduinoJson.h>

// =========================================================================
// USER CONFIGURATION: Wi-Fi & Server Settings
// =========================================================================
const char *ssid = "iQOO_Z10_X";
const char *password = "iQooZ10x";

const char *serverHost = "10.226.94.110";
const uint16_t serverPort = 8000;
const char *armWsPath = "/api/v1/arm/ws/device";

// =========================================================================
// PCA9685 PWM SERVO DRIVER CONFIGURATION
// =========================================================================
Adafruit_PWMServoDriver pwm = Adafruit_PWMServoDriver(0x40);

#define I2C_SDA 21
#define I2C_SCL 22

#define SERVO1_CHANNEL 0 // Servo 1 (Base Joint)
#define SERVO2_CHANNEL 1 // Servo 2 (Shoulder Joint)
#define SERVO3_CHANNEL 2 // Servo 3 (Wrist Joint)
#define SERVO4_CHANNEL 3 // Servo 4 (Gripper / End-Effector)

#define SERVO_MIN 102 // ~0.5 ms pulse (0 degrees)
#define SERVO_MAX 512 // ~2.5 ms pulse (180 degrees)

// Current Servo Position Memory (0 to 180 degrees)
int currentServo1Angle = 90;
int currentServo2Angle = 90;
int currentServo3Angle = 90;
int currentServo4Angle = 90;

// =========================================================================
// WEBSOCKET CLIENT CONNECTION
// =========================================================================
WiFiClient wsClientArm;
bool wsArmConnected = false;

// =========================================================================
// SERVO HARDWARE CONTROL FUNCTIONS
// =========================================================================
void setServoAngle(uint8_t channel, int angle) {
  angle = constrain(angle, 0, 180);
  int pulse = map(angle, 0, 180, SERVO_MIN, SERVO_MAX);
  pwm.setPWM(channel, 0, pulse);
}

void setupServos() {
  Wire.begin(I2C_SDA, I2C_SCL);
  pwm.begin();
  pwm.setPWMFreq(50); // 50 Hz frequency for standard RC servos

  setServoAngle(SERVO1_CHANNEL, currentServo1Angle);
  setServoAngle(SERVO2_CHANNEL, currentServo2Angle);
  setServoAngle(SERVO3_CHANNEL, currentServo3Angle);
  setServoAngle(SERVO4_CHANNEL, currentServo4Angle);

  Serial.printf("[SERVO] PCA9685 Servo Driver Initialized on I2C (SDA=%d, SCL=%d) at 50Hz!\n", I2C_SDA, I2C_SCL);
  Serial.println("[SERVO] Home positions set to 90° for all 4 servos.");
}

void updateServos(int s1, int s2, int s3, int s4) {
  s1 = constrain(s1, 0, 180);
  s2 = constrain(s2, 0, 180);
  s3 = constrain(s3, 0, 180);
  s4 = constrain(s4, 0, 180);

  if (s1 != currentServo1Angle) {
    currentServo1Angle = s1;
    setServoAngle(SERVO1_CHANNEL, s1);
    Serial.printf("🤖 [ARM] Servo 1 (Base) -> %d°\n", s1);
  }
  if (s2 != currentServo2Angle) {
    currentServo2Angle = s2;
    setServoAngle(SERVO2_CHANNEL, s2);
    Serial.printf("🤖 [ARM] Servo 2 (Shoulder) -> %d°\n", s2);
  }
  if (s3 != currentServo3Angle) {
    currentServo3Angle = s3;
    setServoAngle(SERVO3_CHANNEL, s3);
    Serial.printf("🤖 [ARM] Servo 3 (Wrist) -> %d°\n", s3);
  }
  if (s4 != currentServo4Angle) {
    currentServo4Angle = s4;
    setServoAngle(SERVO4_CHANNEL, s4);
    Serial.printf("🤖 [ARM] Servo 4 (Gripper) -> %d°\n", s4);
  }
}

// =========================================================================
// WIFI CONNECTION
// =========================================================================
void connectWiFi() {
  Serial.print("[WIFI] Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  WiFi.setSleep(false); // Lowest latency

  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED) {
    delay(200);
    Serial.print(".");
    attempts++;
    if (attempts > 50) {
      Serial.println("\n[WARNING] Wi-Fi connection taking long, retrying...");
      WiFi.begin(ssid, password);
      attempts = 0;
    }
  }

  Serial.println("\n[OK] Wi-Fi Connected!");
  Serial.print("[WIFI] Local IP Address: ");
  Serial.println(WiFi.localIP());
}

// =========================================================================
// WEBSOCKET CONNECTION (ARM DEVICE)
// =========================================================================
bool connectWebSocketArm() {
  if (wsArmConnected && wsClientArm.connected()) {
    return true;
  }

  wsClientArm.stop();
  wsArmConnected = false;

  Serial.printf("[WEBSOCKET] Connecting to ws://%s:%d%s (ARM)...\n", serverHost, serverPort, armWsPath);
  if (!wsClientArm.connect(serverHost, serverPort)) {
    Serial.println("[WEBSOCKET] TCP connection failed for ARM!");
    return false;
  }

  wsClientArm.setNoDelay(true);

  // Send HTTP WebSocket Upgrade Handshake
  wsClientArm.printf("GET %s HTTP/1.1\r\n", armWsPath);
  wsClientArm.printf("Host: %s:%d\r\n", serverHost, serverPort);
  wsClientArm.println("Upgrade: websocket");
  wsClientArm.println("Connection: Upgrade");
  wsClientArm.println("Sec-WebSocket-Key: dGhlIHNhbXBsZSBub25jZQ==");
  wsClientArm.println("Sec-WebSocket-Version: 13");
  wsClientArm.println();

  unsigned long timeout = millis();
  bool handshakeOk = false;

  while (millis() - timeout < 3000) {
    if (wsClientArm.available()) {
      String line = wsClientArm.readStringUntil('\n');
      if (line.indexOf("101") != -1) {
        handshakeOk = true;
      }
      if (line == "\r" || line == "") {
        break;
      }
    }
    delay(5);
  }

  if (handshakeOk) {
    wsArmConnected = true;
    Serial.println("[OK] WebSocket Connected for ARM!");
    return true;
  } else {
    Serial.println("[ERROR] WebSocket handshake failed for ARM!");
    wsClientArm.stop();
    return false;
  }
}

void processIncomingArmWsData() {
  if (!wsClientArm.available()) return;

  uint8_t header[2];
  if (wsClientArm.read(header, 2) != 2) return;

  uint8_t opcode = header[0] & 0x0F;
  bool masked = (header[1] & 0x80) != 0;
  uint64_t payloadLen = header[1] & 0x7F;

  if (payloadLen == 126) {
    uint8_t lenBytes[2];
    if (wsClientArm.read(lenBytes, 2) != 2) return;
    payloadLen = (lenBytes[0] << 8) | lenBytes[1];
  } else if (payloadLen == 127) {
    uint8_t lenBytes[8];
    if (wsClientArm.read(lenBytes, 8) != 8) return;
    payloadLen = 0;
    for (int i = 0; i < 8; i++) {
      payloadLen = (payloadLen << 8) | lenBytes[i];
    }
  }

  uint8_t maskKey[4] = {0};
  if (masked) {
    if (wsClientArm.read(maskKey, 4) != 4) return;
  }

  if (payloadLen > 1024) payloadLen = 1024;
  char payload[payloadLen + 1];

  size_t bytesRead = 0;
  while (bytesRead < payloadLen && wsClientArm.connected()) {
    if (wsClientArm.available()) {
      payload[bytesRead] = wsClientArm.read();
      if (masked) payload[bytesRead] ^= maskKey[bytesRead % 4];
      bytesRead++;
    }
  }
  payload[bytesRead] = '\0';

  if (opcode == 0x1 || opcode == 0x0) {
    StaticJsonDocument<256> doc;
    DeserializationError err = deserializeJson(doc, payload);
    if (!err) {
      int s1 = currentServo1Angle;
      int s2 = currentServo2Angle;
      int s3 = currentServo3Angle;
      int s4 = currentServo4Angle;
      bool updated = false;

      if (doc.containsKey("servo1")) { s1 = doc["servo1"].as<int>(); updated = true; }
      if (doc.containsKey("servo2")) { s2 = doc["servo2"].as<int>(); updated = true; }
      if (doc.containsKey("servo3")) { s3 = doc["servo3"].as<int>(); updated = true; }
      if (doc.containsKey("servo4")) { s4 = doc["servo4"].as<int>(); updated = true; }

      if (updated) {
        updateServos(s1, s2, s3, s4);
      }
    }
  } else if (opcode == 0x8) {
    wsArmConnected = false;
    wsClientArm.stop();
  }
}

// =========================================================================
// SETUP & MAIN LOOP
// =========================================================================
void setup() {
  Serial.begin(115200);
  Serial.println("\n==================================================");
  Serial.println("  ESP32 ARM CONTROLLER (Wi-Fi WebSocket + PCA9685)");
  Serial.println("==================================================");

  setupServos();
  connectWiFi();
  connectWebSocketArm();
}

void loop() {
  if (WiFi.status() != WL_CONNECTED) {
    connectWiFi();
  }

  if (!wsArmConnected || !wsClientArm.connected()) {
    connectWebSocketArm();
  }

  // Process incoming servo angle stream from WebSocket
  processIncomingArmWsData();

  delay(5);
}
