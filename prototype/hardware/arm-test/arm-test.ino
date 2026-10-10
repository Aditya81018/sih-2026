/*
 * ESP32 4-Servo Robotic Arm Controller via WebSockets (PCA9685 I2C Driver)
 * ----------------------------------------------------------------------------
 * Connects to local Wi-Fi and persistent FastAPI WebSocket server
 * (/api/v1/arm/ws/device). Listens for live JSON control messages:
 * {"servo1": angle1, "servo2": angle2, "servo3": angle3, "servo4": angle4}
 * Updates 4 Servo motors (0° to 180°) in real-time using Adafruit PCA9685 PWM Servo Driver over I2C.
 *
 * Board in Arduino IDE: "ESP32 Dev Module" (or AI-Thinker ESP32-CAM)
 * Required Libraries:
 *   - Adafruit PWM Servo Driver Library (Search 'Adafruit PWM Servo Driver' in Library Manager)
 *   - ArduinoJson by Benoit Blanchon (Version 6 or 7)
 */

#include <Wire.h>
#include <Adafruit_PWMServoDriver.h>
#include <WiFi.h>
#include <ArduinoJson.h>

// =========================================================================
// USER CONFIGURATION: Wi-Fi & Server Settings (matching camera-test)
// =========================================================================
const char *ssid = "iQOO_Z10_X";
const char *password = "iQooZ10x";

// FastAPI server address and WebSocket endpoint
const char *serverHost = "10.226.94.110";
const uint16_t serverPort = 8000;
const char *wsPath = "/api/v1/arm/ws/device";

// =========================================================================
// PCA9685 PWM SERVO DRIVER CONFIGURATION
// =========================================================================
// Create PCA9685 object with default I2C address 0x40
Adafruit_PWMServoDriver pwm = Adafruit_PWMServoDriver(0x40);

// I2C Pin Assignment on ESP32
#define I2C_SDA 21
#define I2C_SCL 22

// PCA9685 Channel Assignments for 4 Servos
#define SERVO1_CHANNEL 0 // Servo 1 (Base / Joint 1)
#define SERVO2_CHANNEL 1 // Servo 2 (Arm / Joint 2)
#define SERVO3_CHANNEL 2 // Servo 3 (Wrist / Joint 3)
#define SERVO4_CHANNEL 3 // Servo 4 (Gripper / Joint 4)

// Min and Max 12-bit pulse counts for standard 50Hz RC servos (0.5ms - 2.5ms out of 4096)
#define SERVO_MIN 102 // ~0.5 ms (0 degrees)
#define SERVO_MAX 512 // ~2.5 ms (180 degrees)

// Current Servo Position Memory (0 to 180 degrees)
int currentServo1Angle = 90;
int currentServo2Angle = 90;
int currentServo3Angle = 90;
int currentServo4Angle = 90;

// WebSocket Client State
WiFiClient wsClient;
bool wsConnected = false;

// =========================================================================
// SERVO HELPER FUNCTIONS (PCA9685)
// =========================================================================
void setServoAngle(uint8_t channel, int angle) {
  angle = constrain(angle, 0, 180);
  int pulse = map(angle, 0, 180, SERVO_MIN, SERVO_MAX);
  pwm.setPWM(channel, 0, pulse);
}

void setupServos() {
  Wire.begin(I2C_SDA, I2C_SCL);

  pwm.begin();
  pwm.setPWMFreq(50); // 50 Hz frequency for standard servos

  // Set initial home positions (90 degrees)
  setServoAngle(SERVO1_CHANNEL, currentServo1Angle);
  setServoAngle(SERVO2_CHANNEL, currentServo2Angle);
  setServoAngle(SERVO3_CHANNEL, currentServo3Angle);
  setServoAngle(SERVO4_CHANNEL, currentServo4Angle);

  Serial.printf(
      "[SERVO] PCA9685 PWM Driver Initialized on I2C (SDA=%d, SCL=%d) at 50Hz!\n",
      I2C_SDA, I2C_SCL);
  Serial.printf("[SERVO] Ch 0: Servo 1 | Ch 1: Servo 2 | Ch 2: Servo 3 | Ch 3: Servo 4\n");
}

// Update servo angles safely with range checks
void updateServos(int angle1, int angle2, int angle3, int angle4) {
  angle1 = constrain(angle1, 0, 180);
  angle2 = constrain(angle2, 0, 180);
  angle3 = constrain(angle3, 0, 180);
  angle4 = constrain(angle4, 0, 180);

  if (angle1 != currentServo1Angle) {
    currentServo1Angle = angle1;
    setServoAngle(SERVO1_CHANNEL, angle1);
    Serial.printf("🤖 Servo 1 (Ch %d) updated to %d°\n", SERVO1_CHANNEL, angle1);
  }

  if (angle2 != currentServo2Angle) {
    currentServo2Angle = angle2;
    setServoAngle(SERVO2_CHANNEL, angle2);
    Serial.printf("🤖 Servo 2 (Ch %d) updated to %d°\n", SERVO2_CHANNEL, angle2);
  }

  if (angle3 != currentServo3Angle) {
    currentServo3Angle = angle3;
    setServoAngle(SERVO3_CHANNEL, angle3);
    Serial.printf("🤖 Servo 3 (Ch %d) updated to %d°\n", SERVO3_CHANNEL, angle3);
  }

  if (angle4 != currentServo4Angle) {
    currentServo4Angle = angle4;
    setServoAngle(SERVO4_CHANNEL, angle4);
    Serial.printf("🤖 Servo 4 (Ch %d) updated to %d°\n", SERVO4_CHANNEL, angle4);
  }
}

// =========================================================================
// WIFI CONNECTION
// =========================================================================
void connectWiFi() {
  Serial.print("[WIFI] Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  WiFi.setSleep(false); // Disable WiFi sleep for lowest latency

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

  Serial.println("");
  Serial.println("[OK] Wi-Fi Connected!");
  Serial.print("[WIFI] Local IP Address: ");
  Serial.println(WiFi.localIP());
}

// =========================================================================
// WEBSOCKET HANDSHAKE & CONNECTION
// =========================================================================
bool connectWebSocket() {
  if (wsConnected && wsClient.connected()) {
    return true;
  }

  wsClient.stop();
  wsConnected = false;

  Serial.printf("[WEBSOCKET] Connecting to ws://%s:%d%s...\n", serverHost,
                serverPort, wsPath);
  if (!wsClient.connect(serverHost, serverPort)) {
    Serial.println("[WEBSOCKET] TCP connection failed!");
    return false;
  }

  // Disable Nagle's algorithm for instant packet delivery
  wsClient.setNoDelay(true);

  // Send standard HTTP WebSocket Upgrade Handshake
  wsClient.printf("GET %s HTTP/1.1\r\n", wsPath);
  wsClient.printf("Host: %s:%d\r\n", serverHost, serverPort);
  wsClient.println("Upgrade: websocket");
  wsClient.println("Connection: Upgrade");
  wsClient.println("Sec-WebSocket-Key: dGhlIHNhbXBsZSBub25jZQ==");
  wsClient.println("Sec-WebSocket-Version: 13");
  wsClient.println();

  // Wait for 101 Switching Protocols response header
  unsigned long timeout = millis();
  bool handshakeOk = false;

  while (millis() - timeout < 3000) {
    if (wsClient.available()) {
      String line = wsClient.readStringUntil('\n');
      if (line.indexOf("101") != -1) {
        handshakeOk = true;
      }
      if (line == "\r" || line == "") {
        break; // End of HTTP response headers
      }
    }
    delay(5);
  }

  if (handshakeOk) {
    wsConnected = true;
    Serial.println("[OK] WebSocket connected to Robotic Arm Endpoint! "
                   "Listening for live slider commands.");
    return true;
  } else {
    Serial.println("[ERROR] WebSocket handshake failed!");
    wsClient.stop();
    return false;
  }
}

// =========================================================================
// PROCESS INCOMING WEBSOCKET MESSAGES FROM SERVER
// =========================================================================
void processIncomingWebSocketData() {
  if (!wsClient.available())
    return;

  // Read WebSocket Frame Header (RFC 6455)
  uint8_t header[2];
  if (wsClient.read(header, 2) != 2)
    return;

  bool fin = (header[0] & 0x80) != 0;
  uint8_t opcode = header[0] & 0x0F;
  bool masked = (header[1] & 0x80) != 0;
  uint64_t payloadLen = header[1] & 0x7F;

  if (payloadLen == 126) {
    uint8_t lenBytes[2];
    if (wsClient.read(lenBytes, 2) != 2)
      return;
    payloadLen = (lenBytes[0] << 8) | lenBytes[1];
  } else if (payloadLen == 127) {
    uint8_t lenBytes[8];
    if (wsClient.read(lenBytes, 8) != 8)
      return;
    payloadLen = 0;
    for (int i = 0; i < 8; i++) {
      payloadLen = (payloadLen << 8) | lenBytes[i];
    }
  }

  uint8_t maskKey[4] = {0};
  if (masked) {
    if (wsClient.read(maskKey, 4) != 4)
      return;
  }

  // Read payload buffer
  if (payloadLen > 1024)
    payloadLen = 1024; // Protect memory limit
  char payload[payloadLen + 1];

  size_t bytesRead = 0;
  while (bytesRead < payloadLen && wsClient.connected()) {
    if (wsClient.available()) {
      payload[bytesRead] = wsClient.read();
      if (masked) {
        payload[bytesRead] ^= maskKey[bytesRead % 4];
      }
      bytesRead++;
    }
  }
  payload[bytesRead] = '\0';

  // Opcode 0x1 = Text Frame (JSON)
  if (opcode == 0x1 || opcode == 0x0) {
    // Parse JSON payload using ArduinoJson
    StaticJsonDocument<256> doc;
    DeserializationError err = deserializeJson(doc, payload);

    if (!err) {
      int s1 = currentServo1Angle;
      int s2 = currentServo2Angle;
      int s3 = currentServo3Angle;
      int s4 = currentServo4Angle;
      bool hasUpdate = false;

      if (doc.containsKey("servo1")) {
        s1 = doc["servo1"].as<int>();
        hasUpdate = true;
      }
      if (doc.containsKey("servo2")) {
        s2 = doc["servo2"].as<int>();
        hasUpdate = true;
      }
      if (doc.containsKey("servo3")) {
        s3 = doc["servo3"].as<int>();
        hasUpdate = true;
      }
      if (doc.containsKey("servo4")) {
        s4 = doc["servo4"].as<int>();
        hasUpdate = true;
      }

      if (hasUpdate) {
        updateServos(s1, s2, s3, s4);
      }
    }
  } else if (opcode == 0x8) { // Connection Close
    Serial.println("[WEBSOCKET] Server requested connection close.");
    wsConnected = false;
    wsClient.stop();
  }
}

// =========================================================================
// SETUP & MAIN LOOP
// =========================================================================
void setup() {
  Serial.begin(115200);
  Serial.println("\n==================================================");
  Serial.println("  ESP32 PCA9685 4-Servo Arm Controller Start");
  Serial.println("==================================================");

  setupServos();
  connectWiFi();
  connectWebSocket();
}

void loop() {
  if (WiFi.status() != WL_CONNECTED) {
    connectWiFi();
  }

  if (!wsConnected || !wsClient.connected()) {
    if (!connectWebSocket()) {
      delay(500);
      return;
    }
  }

  // Process incoming slider angle packets from WebSocket
  processIncomingWebSocketData();

  delay(5); // Smooth execution loop
}
