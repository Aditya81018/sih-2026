/*
 * ESP32-CAM Ultra-High-FPS Real-Time WebSocket Camera Streamer
 * -----------------------------------------------------------
 * Optimized for MAXIMUM FPS (30-60+ FPS) over WebSockets.
 * Resolution: QVGA (320x240) / Full FOV Downsampled.
 * JPEG Quality: Optimized for small byte size and instant transmission.
 *
 * Board in Arduino IDE: "AI Thinker ESP32-CAM"
 */

#include "esp_camera.h"
#include "img_converters.h"
#include <WiFi.h>
#include "camera_pins.h"

// =========================================================================
// USER CONFIGURATION: Update Wi-Fi and Server settings here
// =========================================================================
const char* ssid         = "iQOO_Z10_X";
const char* password     = "iQooZ10x";

// FastAPI server address
const char* serverHost   = "10.195.215.110";
const uint16_t serverPort = 8000;
const char* wsPath       = "/api/v1/camera/ws/ingest";

// Frame delay in ms (0ms = maximum possible FPS)
const unsigned long frameDelayMs = 0;

// Flash LED configuration (Pin 4 on AI-Thinker). Set false for maximum speed
const bool useFlashLed   = false;
// =========================================================================

WiFiClient wsClient;
bool wsConnected = false;

// =========================================================================
// CAMERA INITIALIZATION (OPTIMIZED FOR MAXIMUM FPS)
// =========================================================================
void setupCamera() {
  camera_config_t config;
  config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer   = LEDC_TIMER_0;
  config.pin_d0       = Y2_GPIO_NUM;
  config.pin_d1       = Y3_GPIO_NUM;
  config.pin_d2       = Y4_GPIO_NUM;
  config.pin_d3       = Y5_GPIO_NUM;
  config.pin_d4       = Y6_GPIO_NUM;
  config.pin_d5       = Y7_GPIO_NUM;
  config.pin_d6       = Y8_GPIO_NUM;
  config.pin_d7       = Y9_GPIO_NUM;
  config.pin_xclk     = XCLK_GPIO_NUM;
  config.pin_pclk     = PCLK_GPIO_NUM;
  config.pin_vsync    = VSYNC_GPIO_NUM;
  config.pin_href     = HREF_GPIO_NUM;
  config.pin_sccb_sda = SIOD_GPIO_NUM;
  config.pin_sccb_scl = SIOC_GPIO_NUM;
  config.pin_pwdn     = PWDN_GPIO_NUM;
  config.pin_reset    = RESET_GPIO_NUM;
  config.xclk_freq_hz = 20000000;            // 20MHz sensor clock
  config.pixel_format = PIXFORMAT_JPEG;
  config.grab_mode    = CAMERA_GRAB_LATEST;

  // Ultra High FPS configuration using QVGA (320x240)
  if (psramFound()) {
    config.frame_size   = FRAMESIZE_QVGA;   // 320x240 for 30-60 FPS max throughput
    config.jpeg_quality = 20;              // 20-25: smaller payload size = higher FPS
    config.fb_count     = 2;               // Double buffering for zero wait time
    config.fb_location  = CAMERA_FB_IN_PSRAM;
  } else {
    config.frame_size   = FRAMESIZE_QVGA;
    config.jpeg_quality = 22;
    config.fb_count     = 2;
    config.fb_location  = CAMERA_FB_IN_DRAM;
  }

  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    Serial.printf("[WARNING] Camera JPEG init returned 0x%x. Retrying with RGB565 fallback...\n", err);
    config.pixel_format = PIXFORMAT_RGB565;
    err = esp_camera_init(&config);
  }

  if (err != ESP_OK) {
    Serial.printf("[ERROR] Camera init failed with error 0x%x\n", err);
    while (true) { delay(1000); }
  }

  sensor_t *s = esp_camera_sensor_get();
  if (s) {
    s->set_dcw(s, 1);                     // Downsize Enable: Scale full sensor array (Full 4:3 FOV)
    s->set_framesize(s, FRAMESIZE_QVGA);  // 320x240 High FPS mode
    s->set_quality(s, 20);                // Fast hardware compression
  }

  Serial.println("[OK] Camera initialized for MAXIMUM FPS (QVGA 320x240)!");
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

  Serial.printf("[WEBSOCKET] Connecting to ws://%s:%d%s...\n", serverHost, serverPort, wsPath);
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
    Serial.println("[OK] WebSocket connected and handshake complete! Ready for Ultra-High FPS.");
    return true;
  } else {
    Serial.println("[ERROR] WebSocket handshake failed!");
    wsClient.stop();
    return false;
  }
}

// =========================================================================
// SEND BINARY FRAME OVER WEBSOCKET
// =========================================================================
bool sendWebSocketBinaryFrame(uint8_t* payload, size_t length) {
  if (!wsClient.connected()) {
    wsConnected = false;
    return false;
  }

  // Build RFC 6455 Binary Frame Header (Opcode 0x2, Masked=1)
  uint8_t header[14];
  size_t headerSize = 0;

  header[0] = 0x82; // FIN = 1, Opcode = 0x2 (Binary Frame)

  // Zero mask key so payload XOR 0x00000000 does not mutate bytes
  uint8_t maskKey[4] = {0x00, 0x00, 0x00, 0x00};

  if (length < 126) {
    header[1] = 0x80 | (uint8_t)length;
    memcpy(&header[2], maskKey, 4);
    headerSize = 6;
  } else if (length <= 65535) {
    header[1] = 0x80 | 126;
    header[2] = (uint8_t)((length >> 8) & 0xFF);
    header[3] = (uint8_t)(length & 0xFF);
    memcpy(&header[4], maskKey, 4);
    headerSize = 8;
  } else {
    header[1] = 0x80 | 127;
    header[2] = 0; header[3] = 0; header[4] = 0; header[5] = 0;
    header[6] = (uint8_t)((length >> 24) & 0xFF);
    header[7] = (uint8_t)((length >> 16) & 0xFF);
    header[8] = (uint8_t)((length >> 8) & 0xFF);
    header[9] = (uint8_t)(length & 0xFF);
    memcpy(&header[10], maskKey, 4);
    headerSize = 14;
  }

  // Send frame header + binary JPEG payload
  size_t sentHeader = wsClient.write(header, headerSize);
  size_t sentPayload = wsClient.write(payload, length);

  if (sentHeader == headerSize && sentPayload == length) {
    return true;
  } else {
    Serial.println("[WEBSOCKET] Frame send error, reconnecting...");
    wsConnected = false;
    wsClient.stop();
    return false;
  }
}

// =========================================================================
// SETUP & LOOP
// =========================================================================
void setup() {
  Serial.begin(115200);
  Serial.println("\n==========================================");
  Serial.println("  ESP32-CAM High-FPS WebSocket Streamer Start");
  Serial.println("==========================================");

#if defined(LED_GPIO_NUM)
  if (useFlashLed) {
    pinMode(LED_GPIO_NUM, OUTPUT);
    digitalWrite(LED_GPIO_NUM, LOW);
  }
#endif

  setupCamera();
  connectWiFi();
  connectWebSocket();
}

void loop() {
  if (WiFi.status() != WL_CONNECTED) {
    connectWiFi();
  }

  if (!wsConnected || !wsClient.connected()) {
    if (!connectWebSocket()) {
      delay(200);
      return;
    }
  }

  // Flush incoming control frames quickly if any
  while (wsClient.available()) {
    wsClient.read();
  }

  // Capture frame
  camera_fb_t* fb = esp_camera_fb_get();
  if (!fb) {
    Serial.println("[ERROR] Frame capture failed");
    delay(20);
    return;
  }

  uint8_t* jpegBuf = NULL;
  size_t jpegLen = 0;
  bool isConverted = false;

  if (fb->format == PIXFORMAT_JPEG) {
    jpegBuf = fb->buf;
    jpegLen = fb->len;
  } else {
    isConverted = frame2jpg(fb, 80, &jpegBuf, &jpegLen);
    if (!isConverted) {
      esp_camera_fb_return(fb);
      delay(20);
      return;
    }
  }

  // High-speed WebSocket streaming
  sendWebSocketBinaryFrame(jpegBuf, jpegLen);

  if (isConverted && jpegBuf) {
    free(jpegBuf);
  }

  esp_camera_fb_return(fb);

  if (frameDelayMs > 0) {
    delay(frameDelayMs);
  }
}
