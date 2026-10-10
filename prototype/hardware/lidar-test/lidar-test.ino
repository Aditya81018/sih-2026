/*
 * ESP32 275° Sonar Radar / LIDAR Scanner
 * ----------------------------------------------------------------------------
 * Components:
 *   1. 28BYJ-48 5V Stepper Motor + ULN2003 Driver Board (IN1, IN2, IN3, IN4)
 *   2. HC-SR04 Ultrasonic Sonar Distance Sensor (Trig, Echo)
 *
 * Features:
 *   - Dynamic Step Angle / Speed Multiplier: 1°, 2°, 4°, 5° resolution selectable live!
 *   - Fast 4-step full stepping sequence (2048 steps per 360° output shaft turn).
 *   - Sweeps continuously back and forth: 0° -> 275° (CW), then 275° -> 0° (CCW).
 *   - Stream live readings (angle, distance, sweep direction) to the server
 *     via WebSockets using RFC 6455 compliant frame masking.
 *
 * Board in Arduino IDE: "ESP32 Dev Module"
 * Required Libraries:
 *   - ArduinoJson by Benoit Blanchon (Version 6 or 7)
 *   - WiFi (Built-in ESP32)
 */

#include <WiFi.h>
#include <ArduinoJson.h>

// =========================================================================
// USER CONFIGURATION: Wi-Fi & Server Settings
// =========================================================================
const char *ssid = "iQOO_Z10_X";
const char *password = "iQooZ10x";

// FastAPI server address and WebSocket endpoint
const char *serverHost = "10.226.94.110";
const uint16_t serverPort = 8000;
const char *wsPath = "/api/v1/lidar/ws/device";

// =========================================================================
// PIN DEFINITIONS (ESP32 GPIO Pins)
// =========================================================================
// ULN2003 Stepper Motor Driver Pins
#define IN1_PIN 19
#define IN2_PIN 18
#define IN3_PIN 5
#define IN4_PIN 17

// HC-SR04 Sonar Sensor Pins
#define TRIG_PIN 23
#define ECHO_PIN 22

// =========================================================================
// SCANNER CONFIGURATION & MOTOR PARAMETERS
// =========================================================================
const int SWEEP_RANGE_DEG = 275;     // Total scanning angle range (0° to 275°)
int stepAngle = 2;                   // Dynamic step resolution in degrees (1°, 2°, 4°, 5°)

// 28BYJ-48 5V Stepper Motor Full-Stepping Math (4 steps per cycle = 2048 steps per 360°)
const float STEPS_PER_DEGREE = 2048.0 / 360.0; // ~5.6888 steps/degree

// Motor step delay in microseconds (controls full-step motor speed)
const int FULL_STEP_DELAY_US = 1400;

// Current state variables
int currentAngle = 0;           // Current position in degrees (0 to 275)
bool sweepClockwise = true;      // true = 0 -> 275°, false = 275 -> 0°
long currentStepPosition = 0;    // Motor absolute step count
bool scannerActive = true;       // Scan enable/pause toggle

// WebSocket Client State
WiFiClient wsClient;
bool wsConnected = false;
unsigned long lastReconnectAttempt = 0;

// Full-stepping lookup table for high torque & double rotation speed
const int FULL_STEPS[4][4] = {
  {1, 1, 0, 0},
  {0, 1, 1, 0},
  {0, 0, 1, 1},
  {1, 0, 0, 1}
};

int stepSequenceIndex = 0;

// =========================================================================
// FUNCTION PROTOTYPES
// =========================================================================
void connectWiFi();
bool connectWebSocket();
void handleWebSocketData();
void sendWebSocketFrame(const String &payload);
void stepMotorToAngle(int targetAngle);
void executeFullStep(int dir);
void disableMotorCoils();
float readSonarDistanceCM();
void sendLidarData(int angle, float distance, bool isCW);

// =========================================================================
// SETUP & INITIALIZATION
// =========================================================================
void setup() {
  Serial.begin(115200);
  delay(500);

  Serial.println("\n==============================================");
  Serial.println("  ESP32 High-Speed 275° Sonar Radar Scanner  ");
  Serial.println("==============================================");

  // Configure Stepper Motor Pins
  pinMode(IN1_PIN, OUTPUT);
  pinMode(IN2_PIN, OUTPUT);
  pinMode(IN3_PIN, OUTPUT);
  pinMode(IN4_PIN, OUTPUT);
  disableMotorCoils();

  // Configure Sonar Sensor Pins
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
  digitalWrite(TRIG_PIN, LOW);

  // Connect to Wi-Fi
  connectWiFi();

  // Calibrate stepper to home position (0°)
  currentAngle = 0;
  currentStepPosition = 0;

  Serial.println("[SYSTEM] Hardware initialization complete!");
  Serial.printf("[SYSTEM] Motor Pins: IN1=%d, IN2=%d, IN3=%d, IN4=%d\n", IN1_PIN, IN2_PIN, IN3_PIN, IN4_PIN);
  Serial.printf("[SYSTEM] Sonar Pins: TRIG=%d, ECHO=%d\n", TRIG_PIN, ECHO_PIN);
  Serial.printf("[SYSTEM] Sweep Range: 0° to %d° | Default Step Resolution: %d°\n", SWEEP_RANGE_DEG, stepAngle);
}

// =========================================================================
// MAIN LOOP
// =========================================================================
void loop() {
  // Maintain Wi-Fi connection
  if (WiFi.status() != WL_CONNECTED) {
    connectWiFi();
    return;
  }

  // Maintain WebSocket connection
  if (!wsConnected || !wsClient.connected()) {
    wsConnected = false;
    if (millis() - lastReconnectAttempt > 2000) {
      lastReconnectAttempt = millis();
      connectWebSocket();
    }
  } else {
    handleWebSocketData();
  }

  // Execute scan step if active
  if (scannerActive) {
    // 1. Calculate target angle for next step using stepAngle multiplier
    if (sweepClockwise) {
      currentAngle += stepAngle;
      if (currentAngle >= SWEEP_RANGE_DEG) {
        currentAngle = SWEEP_RANGE_DEG;
        sweepClockwise = false; // Reverse direction to CCW
        Serial.println("🔄 [SCANNER] Reached 275° limit. Reversing to CCW");
      }
    } else {
      currentAngle -= stepAngle;
      if (currentAngle <= 0) {
        currentAngle = 0;
        sweepClockwise = true; // Reverse direction to CW
        Serial.println("🔄 [SCANNER] Reached 0° limit. Reversing to CW");
      }
    }

    // 2. Rotate motor to target angle position
    stepMotorToAngle(currentAngle);

    // Short stabilization pause before acoustic pulse
    delayMicroseconds(1500);

    // 3. Measure distance with HC-SR04 sonar
    float distance = readSonarDistanceCM();

    // 4. Transmit scan telemetry over WebSocket
    sendLidarData(currentAngle, distance, sweepClockwise);
  } else {
    // Scanner paused, release motor coils to avoid overheating
    disableMotorCoils();
    delay(50);
  }
}

// =========================================================================
// MOTOR CONTROL FUNCTIONS (28BYJ-48 + ULN2003 Full Stepping)
// =========================================================================

// Moves the stepper motor precisely to target step position calculated for targetAngle
void stepMotorToAngle(int targetAngle) {
  long targetStep = round(targetAngle * STEPS_PER_DEGREE);
  long stepDiff = targetStep - currentStepPosition;

  int dir = (stepDiff > 0) ? 1 : -1;
  long stepsToTake = abs(stepDiff);

  for (long i = 0; i < stepsToTake; i++) {
    executeFullStep(dir);
    currentStepPosition += dir;
    delayMicroseconds(FULL_STEP_DELAY_US);
  }
}

// Powers coils for next step in sequence
void executeFullStep(int dir) {
  stepSequenceIndex += dir;
  if (stepSequenceIndex > 3) stepSequenceIndex = 0;
  if (stepSequenceIndex < 0) stepSequenceIndex = 3;

  digitalWrite(IN1_PIN, FULL_STEPS[stepSequenceIndex][0]);
  digitalWrite(IN2_PIN, FULL_STEPS[stepSequenceIndex][1]);
  digitalWrite(IN3_PIN, FULL_STEPS[stepSequenceIndex][2]);
  digitalWrite(IN4_PIN, FULL_STEPS[stepSequenceIndex][3]);
}

// Disables all motor driver outputs to prevent current draw & heat while stationary
void disableMotorCoils() {
  digitalWrite(IN1_PIN, LOW);
  digitalWrite(IN2_PIN, LOW);
  digitalWrite(IN3_PIN, LOW);
  digitalWrite(IN4_PIN, LOW);
}

// =========================================================================
// SONAR SENSOR FUNCTION (HC-SR04)
// =========================================================================
float readSonarDistanceCM() {
  // Clear trigger pin
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);

  // Send 10us HIGH pulse
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);

  // Measure pulse duration on ECHO pin (10ms timeout ≈ 170 cm max distance)
  unsigned long duration = pulseIn(ECHO_PIN, HIGH, 10000);

  if (duration == 0) {
    return -1.0; // Out of range or no echo returned
  }

  // Speed of sound = 343 m/s = 0.0343 cm/us
  float distanceCM = (duration * 0.0343) / 2.0;

  // Filter invalid boundary readings
  if (distanceCM < 2.0 || distanceCM > 350.0) {
    return -1.0;
  }

  return distanceCM;
}

// =========================================================================
// WI-FI & WEBSOCKET FUNCTIONS (RFC 6455 Compliant)
// =========================================================================
void connectWiFi() {
  if (WiFi.status() == WL_CONNECTED) return;

  Serial.printf("\n[WIFI] Connecting to SSID: %s...", ssid);
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  int attempt = 0;
  while (WiFi.status() != WL_CONNECTED && attempt < 30) {
    delay(500);
    Serial.print(".");
    attempt++;
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\n[WIFI] Connected successfully!");
    Serial.printf("[WIFI] ESP32 IP Address: %s\n", WiFi.localIP().toString().c_str());
  } else {
    Serial.println("\n[WIFI] Connection failed. Will retry...");
  }
}

bool connectWebSocket() {
  Serial.printf("[WS] Connecting to WebSocket Server at %s:%d%s...\n", serverHost, serverPort, wsPath);
  wsClient.stop();
  wsClient.setNoDelay(true);

  if (wsClient.connect(serverHost, serverPort)) {
    // Send HTTP upgrade request to initiate WebSocket connection
    wsClient.printf("GET %s HTTP/1.1\r\n", wsPath);
    wsClient.printf("Host: %s:%d\r\n", serverHost, serverPort);
    wsClient.println("Upgrade: websocket");
    wsClient.println("Connection: Upgrade");
    wsClient.println("Sec-WebSocket-Key: dGhlIHNhbXBsZSBub25jZQ==");
    wsClient.println("Sec-WebSocket-Version: 13");
    wsClient.println();

    // Check HTTP response header
    unsigned long timeout = millis();
    bool handshakeOk = false;

    while (millis() - timeout < 3000) {
      if (wsClient.available()) {
        String line = wsClient.readStringUntil('\n');
        if (line.indexOf("101") != -1) { // 101 Switching Protocols
          handshakeOk = true;
        }
        if (line == "\r" || line == "") { // End of headers
          break;
        }
      }
      delay(5);
    }

    if (handshakeOk) {
      wsConnected = true;
      Serial.println("🟢 [WS] WebSocket connected & upgraded!");
      return true;
    }
  }

  wsConnected = false;
  wsClient.stop();
  return false;
}

// RFC 6455 Compliant Client-to-Server Masked Frame Sender
void sendWebSocketFrame(const String &payload) {
  if (!wsConnected || !wsClient.connected()) return;

  size_t len = payload.length();
  uint8_t maskKey[4] = {0x12, 0x34, 0x56, 0x78}; // Client 4-byte mask key

  wsClient.write(0x81); // Text frame

  if (len <= 125) {
    wsClient.write(0x80 | (uint8_t)len);
  } else {
    wsClient.write(0x80 | 126);
    wsClient.write((uint8_t)(len >> 8));
    wsClient.write((uint8_t)(len & 0xFF));
  }

  wsClient.write(maskKey, 4);

  for (size_t i = 0; i < len; i++) {
    wsClient.write(payload[i] ^ maskKey[i % 4]);
  }
}

void handleWebSocketData() {
  if (!wsClient.connected()) {
    wsConnected = false;
    return;
  }

  while (wsClient.available() >= 2) {
    uint8_t header[2];
    if (wsClient.read(header, 2) != 2) return;

    uint8_t opcode = header[0] & 0x0F;
    bool masked = (header[1] & 0x80) != 0;
    uint64_t payloadLen = header[1] & 0x7F;

    if (payloadLen == 126) {
      if (wsClient.available() < 2) return;
      uint8_t lenBytes[2];
      wsClient.read(lenBytes, 2);
      payloadLen = (lenBytes[0] << 8) | lenBytes[1];
    } else if (payloadLen == 127) {
      if (wsClient.available() < 8) return;
      uint8_t lenBytes[8];
      wsClient.read(lenBytes, 8);
      payloadLen = 0;
      for (int i = 0; i < 8; i++) {
        payloadLen = (payloadLen << 8) | lenBytes[i];
      }
    }

    uint8_t maskKey[4] = {0};
    if (masked) {
      if (wsClient.available() < 4) return;
      wsClient.read(maskKey, 4);
    }

    if (payloadLen > 1024) payloadLen = 1024;
    char payload[payloadLen + 1];
    size_t bytesRead = 0;
    while (bytesRead < payloadLen && wsClient.connected()) {
      if (wsClient.available()) {
        uint8_t b = wsClient.read();
        if (masked) b ^= maskKey[bytesRead % 4];
        payload[bytesRead++] = b;
      }
    }
    payload[bytesRead] = '\0';

    if (opcode == 0x8) {
      wsConnected = false;
      wsClient.stop();
      return;
    } else if (opcode == 0x9) {
      uint8_t pongHeader[6] = {0x8A, 0x80, 0x12, 0x34, 0x56, 0x78};
      wsClient.write(pongHeader, 6);
    } else if (opcode == 0x1 || opcode == 0x0) {
      if (bytesRead > 0 && payload[0] == '{') {
        StaticJsonDocument<256> doc;
        DeserializationError err = deserializeJson(doc, payload);
        if (!err) {
          if (doc.containsKey("active")) {
            scannerActive = doc["active"].as<bool>();
            Serial.printf("🎮 [WS COMMAND] Scanner active set to: %s\n", scannerActive ? "ACTIVE" : "PAUSED");
          }
          if (doc.containsKey("step_angle")) {
            stepAngle = doc["step_angle"].as<int>();
            stepAngle = constrain(stepAngle, 1, 10);
            Serial.printf("🎮 [WS COMMAND] Step angle resolution set to: %d°\n", stepAngle);
          }
          if (doc.containsKey("reset") && doc["reset"].as<bool>()) {
            stepMotorToAngle(0);
            currentAngle = 0;
            sweepClockwise = true;
            Serial.println("🎮 [WS COMMAND] Scanner reset to 0° home position.");
          }
        }
      }
    }
  }
}

void sendLidarData(int angle, float distance, bool isCW) {
  if (!wsConnected || !wsClient.connected()) return;

  StaticJsonDocument<256> doc;
  doc["angle"] = angle;
  if (distance < 0) {
    doc["distance"] = nullptr;
  } else {
    doc["distance"] = distance;
  }
  doc["sweep_direction"] = isCW ? "CW" : "CCW";
  doc["timestamp"] = millis();

  String payload;
  serializeJson(doc, payload);

  sendWebSocketFrame(payload);
}
