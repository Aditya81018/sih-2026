/*
 * ESP32 Robot Car Controller via Wi-Fi & WebSockets
 * ------------------------------------------------------------------------------------
 * Project: SIH2026 Integrated Robot Controller
 * Component: 2-Wheel Robot Car (Independent Wi-Fi WebSocket Client)
 * Location: hardware/robot-test/car-master/car-master.ino
 *
 * System Role:
 *   - Connects directly to Wi-Fi.
 *   - Maintains persistent WebSocket client connection to FastAPI server:
 *       /api/v1/car/ws/device  --> Direction ("FORWARD", "BACKWARD", "LEFT", "RIGHT", "SPIN_LEFT", "SPIN_RIGHT", "STOP") & PWM Speed (0-255)
 *   - Controls 2 DC Motors via L298N Dual H-Bridge Driver using PWM.
 *   - Operates independently from Robotic Arm ESP32 over Wi-Fi (No SPI required).
 *
 * L298N Motor Pins:
 *   - Left Motor : ENA=14, IN1=27, IN2=26
 *   - Right Motor: IN3=25, IN4=33, ENB=32
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

const char *serverHost = "10.226.94.110";
const uint16_t serverPort = 8000;
const char *carWsPath = "/api/v1/car/ws/device";

// =========================================================================
// L298N MOTOR DRIVER PIN ASSIGNMENTS
// =========================================================================
#define ENA_PIN 14  // Left Motor Speed PWM (0-255)
#define IN1_PIN 27  // Left Motor Direction A
#define IN2_PIN 26  // Left Motor Direction B

#define IN3_PIN 25  // Right Motor Direction A
#define IN4_PIN 33  // Right Motor Direction B
#define ENB_PIN 32  // Right Motor Speed PWM (0-255)

// Current Car Motion State
String currentDirection = "STOP";
int currentSpeed = 150; // Default speed (0 to 255 PWM)

// Safety Watchdog: Auto-stop car if server loses communication
unsigned long lastCarCommandTime = 0;
const unsigned long SAFETY_TIMEOUT_MS = 3000; // 3 seconds timeout

// =========================================================================
// WEBSOCKET CLIENT CONNECTION
// =========================================================================
WiFiClient wsClientCar;
bool wsCarConnected = false;

// =========================================================================
// MOTOR CONTROL FUNCTIONS (L298N)
// =========================================================================
void stopMotors() {
  digitalWrite(IN1_PIN, LOW);
  digitalWrite(IN2_PIN, LOW);
  digitalWrite(IN3_PIN, LOW);
  digitalWrite(IN4_PIN, LOW);
  analogWrite(ENA_PIN, 0);
  analogWrite(ENB_PIN, 0);
  currentDirection = "STOP";
}

void setupMotors() {
  pinMode(ENA_PIN, OUTPUT);
  pinMode(IN1_PIN, OUTPUT);
  pinMode(IN2_PIN, OUTPUT);

  pinMode(IN3_PIN, OUTPUT);
  pinMode(IN4_PIN, OUTPUT);
  pinMode(ENB_PIN, OUTPUT);

  stopMotors();
  Serial.println("[MOTORS] L298N Dual Motor Driver Initialized!");
}

void setMotors(int leftSpeed, bool leftForward, int rightSpeed, bool rightForward) {
  leftSpeed = constrain(leftSpeed, 0, 255);
  rightSpeed = constrain(rightSpeed, 0, 255);

  if (leftSpeed > 0) {
    digitalWrite(IN1_PIN, leftForward ? HIGH : LOW);
    digitalWrite(IN2_PIN, leftForward ? LOW : HIGH);
  } else {
    digitalWrite(IN1_PIN, LOW);
    digitalWrite(IN2_PIN, LOW);
  }

  if (rightSpeed > 0) {
    digitalWrite(IN3_PIN, rightForward ? HIGH : LOW);
    digitalWrite(IN4_PIN, rightForward ? LOW : HIGH);
  } else {
    digitalWrite(IN3_PIN, LOW);
    digitalWrite(IN4_PIN, LOW);
  }

  analogWrite(ENA_PIN, leftSpeed);
  analogWrite(ENB_PIN, rightSpeed);
}

void executeMotion(String direction, int speed) {
  speed = constrain(speed, 0, 255);
  currentSpeed = speed;
  currentDirection = direction;
  lastCarCommandTime = millis();

  if (direction == "FORWARD") {
    setMotors(speed, true, speed, true);
    Serial.printf("🏎️ [CAR] FORWARD (Speed: %d)\n", speed);
  } else if (direction == "BACKWARD") {
    setMotors(speed, false, speed, false);
    Serial.printf("🏎️ [CAR] BACKWARD (Speed: %d)\n", speed);
  } else if (direction == "LEFT") {
    int innerSpeed = speed / 3;
    setMotors(innerSpeed, true, speed, true);
    Serial.printf("🏎️ [CAR] TURN LEFT (Left: %d, Right: %d)\n", innerSpeed, speed);
  } else if (direction == "RIGHT") {
    int innerSpeed = speed / 3;
    setMotors(speed, true, innerSpeed, true);
    Serial.printf("🏎️ [CAR] TURN RIGHT (Left: %d, Right: %d)\n", speed, innerSpeed);
  } else if (direction == "SPIN_LEFT") {
    setMotors(speed, false, speed, true);
    Serial.printf("🏎️ [CAR] SPIN LEFT (Speed: %d)\n", speed);
  } else if (direction == "SPIN_RIGHT") {
    setMotors(speed, true, speed, false);
    Serial.printf("🏎️ [CAR] SPIN RIGHT (Speed: %d)\n", speed);
  } else {
    stopMotors();
    Serial.println("🛑 [CAR] Motors STOPPED");
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
// WEBSOCKET CONNECTION (CAR DEVICE)
// =========================================================================
bool connectWebSocketCar() {
  if (wsCarConnected && wsClientCar.connected()) {
    return true;
  }

  wsClientCar.stop();
  wsCarConnected = false;

  Serial.printf("[WEBSOCKET] Connecting to ws://%s:%d%s (CAR)...\n", serverHost, serverPort, carWsPath);
  if (!wsClientCar.connect(serverHost, serverPort)) {
    Serial.println("[WEBSOCKET] TCP connection failed for CAR!");
    return false;
  }

  wsClientCar.setNoDelay(true);

  // Send HTTP WebSocket Upgrade Handshake
  wsClientCar.printf("GET %s HTTP/1.1\r\n", carWsPath);
  wsClientCar.printf("Host: %s:%d\r\n", serverHost, serverPort);
  wsClientCar.println("Upgrade: websocket");
  wsClientCar.println("Connection: Upgrade");
  wsClientCar.println("Sec-WebSocket-Key: dGhlIHNhbXBsZSBub25jZQ==");
  wsClientCar.println("Sec-WebSocket-Version: 13");
  wsClientCar.println();

  unsigned long timeout = millis();
  bool handshakeOk = false;

  while (millis() - timeout < 3000) {
    if (wsClientCar.available()) {
      String line = wsClientCar.readStringUntil('\n');
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
    wsCarConnected = true;
    lastCarCommandTime = millis();
    Serial.println("[OK] WebSocket Connected for CAR!");
    return true;
  } else {
    Serial.println("[ERROR] WebSocket handshake failed for CAR!");
    wsClientCar.stop();
    return false;
  }
}

void processIncomingCarWsData() {
  if (!wsClientCar.available()) return;

  uint8_t header[2];
  if (wsClientCar.read(header, 2) != 2) return;

  uint8_t opcode = header[0] & 0x0F;
  bool masked = (header[1] & 0x80) != 0;
  uint64_t payloadLen = header[1] & 0x7F;

  if (payloadLen == 126) {
    uint8_t lenBytes[2];
    if (wsClientCar.read(lenBytes, 2) != 2) return;
    payloadLen = (lenBytes[0] << 8) | lenBytes[1];
  } else if (payloadLen == 127) {
    uint8_t lenBytes[8];
    if (wsClientCar.read(lenBytes, 8) != 8) return;
    payloadLen = 0;
    for (int i = 0; i < 8; i++) {
      payloadLen = (payloadLen << 8) | lenBytes[i];
    }
  }

  uint8_t maskKey[4] = {0};
  if (masked) {
    if (wsClientCar.read(maskKey, 4) != 4) return;
  }

  if (payloadLen > 1024) payloadLen = 1024;
  char payload[payloadLen + 1];

  size_t bytesRead = 0;
  while (bytesRead < payloadLen && wsClientCar.connected()) {
    if (wsClientCar.available()) {
      payload[bytesRead] = wsClientCar.read();
      if (masked) payload[bytesRead] ^= maskKey[bytesRead % 4];
      bytesRead++;
    }
  }
  payload[bytesRead] = '\0';

  if (opcode == 0x1 || opcode == 0x0) {
    StaticJsonDocument<256> doc;
    DeserializationError err = deserializeJson(doc, payload);
    if (!err) {
      String dir = currentDirection;
      int spd = currentSpeed;
      if (doc.containsKey("direction")) dir = doc["direction"].as<const char *>();
      if (doc.containsKey("speed")) spd = doc["speed"].as<int>();
      executeMotion(dir, spd);
    }
  } else if (opcode == 0x8) {
    wsCarConnected = false;
    stopMotors();
    wsClientCar.stop();
  }
}

// =========================================================================
// SETUP & MAIN LOOP
// =========================================================================
void setup() {
  Serial.begin(115200);
  Serial.println("\n==================================================");
  Serial.println("  ESP32 CAR CONTROLLER (Wi-Fi WebSocket)");
  Serial.println("==================================================");

  setupMotors();
  connectWiFi();
  connectWebSocketCar();
}

void loop() {
  if (WiFi.status() != WL_CONNECTED) {
    stopMotors();
    connectWiFi();
  }

  if (!wsCarConnected || !wsClientCar.connected()) {
    stopMotors();
    connectWebSocketCar();
  }

  // Process incoming control streams from WebSocket
  processIncomingCarWsData();

  // Safety Timeout: Stop car if command lost for > 3s
  if (currentDirection != "STOP" && (millis() - lastCarCommandTime > SAFETY_TIMEOUT_MS)) {
    Serial.println("⚠️ [SAFETY] Car movement command timeout! Stopping motors.");
    stopMotors();
  }

  delay(5);
}
