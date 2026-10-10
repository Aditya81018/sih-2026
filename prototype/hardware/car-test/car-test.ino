/*
  ESP32 LoRa (RA-02) Receiver -> MQTT Gateway
  --------------------------------------------
  Compatible with current ESP32 LoRa node.

  Payload:
    54-byte packed SensorData structure

  LoRa:
    Frequency  : 433 MHz
    Spreading  : SF7
    Sync Word  : 0xF3

  MQTT:
    Publishes the raw 54-byte binary payload.
*/

#include <SPI.h>
#include <LoRa.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include <WiFiClientSecure.h>

// ==================================================
// WiFi credentials
// ==================================================

const char* WIFI_SSID     = "ISP";
const char* WIFI_PASSWORD = "qwerfdsa";

// ==================================================
// MQTT broker settings
// ==================================================

const char* MQTT_BROKER    = "td1cf1ce.ala.asia-southeast1.emqxsl.com";
const int   MQTT_PORT      = 8883;

const char* MQTT_CLIENT_ID = "esp32-lora-gateway";
const char* MQTT_TOPIC     = "sensors/lora/binary";

const char* MQTT_USER      = "aayush";
const char* MQTT_PASS      = "qwerfdsa";

// ==================================================
// Network objects
// ==================================================

WiFiClientSecure espClient;
PubSubClient mqttClient(espClient);

// ==================================================
// LoRa pins
// Must match the transmitter
// ==================================================

#define LORA_SCK   18
#define LORA_MISO  19
#define LORA_MOSI  23
#define LORA_NSS   5
#define LORA_RST   14
#define LORA_DIO0  26

#define LORA_FREQ  433E6

// ==================================================
// Binary Data Structure
// MUST match the node exactly
// Total = 54 bytes
// ==================================================

typedef struct __attribute__((packed)) {

  char id[8];          // 8 bytes

  uint32_t packetSeq;  // 4 bytes

  float temp;          // 4 bytes
  float hum;           // 4 bytes

  float ax;            // 4 bytes
  float ay;            // 4 bytes
  float az;            // 4 bytes

  float gx;            // 4 bytes
  float gy;            // 4 bytes
  float gz;            // 4 bytes

  float dist_cm;       // 4 bytes

  int16_t mq6_raw;     // 2 bytes
  int16_t water_raw;   // 2 bytes
  int16_t pot_raw;     // 2 bytes

} SensorData;

// ==================================================
// Packet tracking
// ==================================================

uint32_t packetsReceived = 0;
uint32_t packetsPublished = 0;

// ==================================================
// Setup
// ==================================================

void setup() {

  Serial.begin(115200);

  while (!Serial) {
    delay(10);
  }

  Serial.println();
  Serial.println("========================================");
  Serial.println(" ESP32 LoRa -> MQTT Gateway");
  Serial.println("========================================");

  setupWiFi();
  setupMQTT();
  setupLoRa();

  Serial.println();
  Serial.println("Gateway ready.");
  Serial.println("Listening for 54-byte LoRa packets...");
  Serial.println("----------------------------------------");
}

// ==================================================
// Main Loop
// ==================================================

void loop() {

  static unsigned long lastHeartbeat = 0;

  // ------------------------------------------------
  // Heartbeat
  // ------------------------------------------------

  if (millis() - lastHeartbeat >= 10000) {

    Serial.println("...still listening for LoRa...");

    lastHeartbeat = millis();
  }

  // ------------------------------------------------
  // Keep WiFi connected
  // ------------------------------------------------

  if (WiFi.status() != WL_CONNECTED) {
    reconnectWiFi();
  }

  // ------------------------------------------------
  // Keep MQTT connected
  // ------------------------------------------------

  if (!mqttClient.connected()) {
    reconnectMQTT();
  }

  mqttClient.loop();

  // ------------------------------------------------
  // Check LoRa
  // ------------------------------------------------

  int packetSize = LoRa.parsePacket();

  if (packetSize == 0) {
    return;
  }

  // ------------------------------------------------
  // Check packet size
  // ------------------------------------------------

  SensorData rxData;

  if (packetSize != sizeof(SensorData)) {

    Serial.println();
    Serial.println("---- Invalid LoRa Packet ----");

    Serial.print("Expected: ");
    Serial.print(sizeof(SensorData));
    Serial.println(" bytes");

    Serial.print("Received: ");
    Serial.print(packetSize);
    Serial.println(" bytes");

    Serial.println("Packet discarded.");
    Serial.println("-----------------------------");

    // Empty the packet
    while (LoRa.available()) {
      LoRa.read();
    }

    return;
  }

  // ------------------------------------------------
  // Read binary payload
  // ------------------------------------------------

  size_t bytesRead = LoRa.readBytes(
    (uint8_t*)&rxData,
    sizeof(rxData)
  );

  if (bytesRead != sizeof(rxData)) {

    Serial.println("ERROR: Failed to read complete LoRa packet.");
    return;
  }

  // ------------------------------------------------
  // LoRa signal information
  // ------------------------------------------------

  int rssi = LoRa.packetRssi();
  float snr = LoRa.packetSnr();

  packetsReceived++;

  // ------------------------------------------------
  // Safely terminate device ID
  // ------------------------------------------------

  char safeId[9];

  memcpy(
    safeId,
    rxData.id,
    8
  );

  safeId[8] = '\0';

  // =================================================
  // Serial output
  // =================================================

  Serial.println();
  Serial.println("========================================");
  Serial.println("       LoRa Packet Received");
  Serial.println("========================================");

  Serial.print("Device ID   : ");
  Serial.println(safeId);

  Serial.print("Packet Seq  : ");
  Serial.println(rxData.packetSeq);

  Serial.print("Packet Size : ");
  Serial.print(sizeof(rxData));
  Serial.println(" bytes");

  Serial.println("----------------------------------------");

  Serial.printf(
    "Temperature : %.2f °C\n",
    rxData.temp
  );

  Serial.printf(
    "Humidity    : %.2f %%\n",
    rxData.hum
  );

  Serial.printf(
    "Accel       : X=%.2f | Y=%.2f | Z=%.2f g\n",
    rxData.ax,
    rxData.ay,
    rxData.az
  );

  Serial.printf(
    "Gyro        : X=%.2f | Y=%.2f | Z=%.2f deg/s\n",
    rxData.gx,
    rxData.gy,
    rxData.gz
  );

  Serial.printf(
    "Distance    : %.2f cm\n",
    rxData.dist_cm
  );

  Serial.printf(
    "MQ6         : %d\n",
    rxData.mq6_raw
  );

  Serial.printf(
    "Water       : %d\n",
    rxData.water_raw
  );

  Serial.printf(
    "Potentiometer: %d\n",
    rxData.pot_raw
  );

  Serial.println("----------------------------------------");

  Serial.print("LoRa RSSI   : ");
  Serial.print(rssi);
  Serial.println(" dBm");

  Serial.print("LoRa SNR    : ");
  Serial.print(snr);
  Serial.println(" dB");

  Serial.print("Packets     : ");
  Serial.println(packetsReceived);

  Serial.println("========================================");

  // =================================================
  // Publish raw binary packet to MQTT
  // =================================================

  publishToMQTT(rxData);
}

// ==================================================
// WiFi Setup
// ==================================================

void setupWiFi() {

  Serial.println();
  Serial.print("Connecting to WiFi: ");
  Serial.println(WIFI_SSID);

  WiFi.mode(WIFI_STA);

  WiFi.begin(
    WIFI_SSID,
    WIFI_PASSWORD
  );

  unsigned long startAttempt = millis();

  while (WiFi.status() != WL_CONNECTED) {

    delay(500);

    Serial.print(".");

    // Prevent infinite blocking
    if (millis() - startAttempt >= 20000) {

      Serial.println();
      Serial.println(
        "WiFi connection timeout."
      );

      Serial.println(
        "Will continue reconnecting in background."
      );

      break;
    }
  }

  if (WiFi.status() == WL_CONNECTED) {

    Serial.println();
    Serial.println("WiFi connected.");

    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());

    // MQTT TLS certificate verification disabled.
    // Suitable for testing.
    espClient.setInsecure();
  }
}

// ==================================================
// WiFi Reconnect
// ==================================================

void reconnectWiFi() {

  static unsigned long lastAttempt = 0;

  if (millis() - lastAttempt < 5000) {
    return;
  }

  lastAttempt = millis();

  Serial.println(
    "WiFi disconnected. Attempting reconnect..."
  );

  WiFi.disconnect();

  WiFi.begin(
    WIFI_SSID,
    WIFI_PASSWORD
  );
}

// ==================================================
// MQTT Setup
// ==================================================

void setupMQTT() {

  mqttClient.setServer(
    MQTT_BROKER,
    MQTT_PORT
  );

  // Increase buffer size to safely handle
  // binary MQTT packets.
  mqttClient.setBufferSize(512);
}

// ==================================================
// MQTT Reconnect
// ==================================================

void reconnectMQTT() {

  static unsigned long lastAttempt = 0;

  if (millis() - lastAttempt < 5000) {
    return;
  }

  lastAttempt = millis();

  if (WiFi.status() != WL_CONNECTED) {
    return;
  }

  Serial.println();
  Serial.print(
    "Connecting to MQTT broker: "
  );

  Serial.println(MQTT_BROKER);

  bool connected;

  if (strlen(MQTT_USER) > 0) {

    connected = mqttClient.connect(
      MQTT_CLIENT_ID,
      MQTT_USER,
      MQTT_PASS
    );

  } else {

    connected = mqttClient.connect(
      MQTT_CLIENT_ID
    );
  }

  if (connected) {

    Serial.println(
      "MQTT connected."
    );

    Serial.print(
      "MQTT topic: "
    );

    Serial.println(MQTT_TOPIC);

  } else {

    Serial.print(
      "MQTT connection failed, state="
    );

    Serial.println(
      mqttClient.state()
    );
  }
}

// ==================================================
// Publish binary sensor data to MQTT
// ==================================================

void publishToMQTT(
  const SensorData &d
) {

  if (!mqttClient.connected()) {

    Serial.println(
      "MQTT not connected - packet not published."
    );

    return;
  }

  // Publish the structure directly as binary.
  //
  // IMPORTANT:
  // MQTT subscribers must interpret this as
  // the same packed 54-byte SensorData structure.

  bool published = mqttClient.publish(
    MQTT_TOPIC,
    (const uint8_t*)&d,
    sizeof(d)
  );

  Serial.println();
  Serial.println("---- MQTT Publish ----");

  Serial.print("Topic   : ");
  Serial.println(MQTT_TOPIC);

  Serial.print("Payload : ");
  Serial.print(sizeof(d));
  Serial.println(" bytes");

  Serial.print("Status  : ");

  if (published) {

    Serial.println("SUCCESS");

    packetsPublished++;

  } else {

    Serial.println("FAILED");
  }

  Serial.print("Published packets: ");
  Serial.println(packetsPublished);

  Serial.println("----------------------");
}

// ==================================================
// LoRa Setup
// ==================================================

void setupLoRa() {

  Serial.println();
  Serial.println("Initializing LoRa...");

  // SPI pins
  SPI.begin(
    LORA_SCK,
    LORA_MISO,
    LORA_MOSI,
    LORA_NSS
  );

  // LoRa control pins
  LoRa.setPins(
    LORA_NSS,
    LORA_RST,
    LORA_DIO0
  );

  // Start LoRa
  if (!LoRa.begin(LORA_FREQ)) {

    Serial.println(
      "LoRa initialization FAILED!"
    );

    Serial.println(
      "Check RA-02 wiring and power."
    );

    while (1) {
      delay(1000);
    }
  }

  // These MUST match the node.
  LoRa.setSpreadingFactor(7);
  LoRa.setSyncWord(0xF3);

  Serial.println(
    "LoRa initialized successfully."
  );

  Serial.println(
    "Frequency   : 433 MHz"
  );

  Serial.println(
    "Spreading   : SF7"
  );

  Serial.println(
    "Sync Word   : 0xF3"
  );

  Serial.print(
    "Payload size: "
  );

  Serial.print(
    sizeof(SensorData)
  );

  Serial.println(
    " bytes"
  );
}