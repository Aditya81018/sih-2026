# ESP32-CAM Live Camera Streamer

This folder contains Arduino code for the **ESP32-CAM** module to capture live camera video frames and stream them continuously to the FastAPI server.

---

## 🛠 Hardware Required

1. **ESP32-CAM** Board (AI-Thinker module)
2. **FTDI Programmer** (USB-to-TTL Serial converter) + Jumper Wires
3. Micro-USB Cable

### Pin Wiring for Flashing (FTDI -> ESP32-CAM)

| FTDI Programmer | ESP32-CAM |
| :--- | :--- |
| **GND** | **GND** |
| **VCC (5V)** | **5V** |
| **TX** | **U0R (GPIO 3)** |
| **RX** | **U0T (GPIO 1)** |
| — | **GPIO 0** connected to **GND** (Flash Mode) |

*Note: Disconnect GPIO 0 from GND after uploading code and press Reset button to run normal mode.*

---

## 📝 Setup Instructions in Arduino IDE

1. **Install ESP32 Board Support**:
   - Open Arduino IDE -> `File` -> `Preferences`.
   - Add to *Additional Boards Manager URLs*:
     `https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json`
   - Open `Tools` -> `Board` -> `Boards Manager...`, search **esp32**, and install **esp32 by Espressif**.

2. **Select Board & Port**:
   - Board: `AI Thinker ESP32-CAM`
   - Partition Scheme: `Huge APP (3MB No OTA/1MB SPIFFS)`
   - PSRAM: `Enabled` (if your board has PSRAM)
   - Port: Select your FTDI Serial Port (e.g. `/dev/ttyUSB0` or `COM3`)

3. **Configure Code**:
   Open `camera-test.ino` and update Wi-Fi SSID, Password, and your Server IP address:
   ```cpp
   const char* ssid     = "YOUR_WIFI_SSID";
   const char* password = "YOUR_WIFI_PASSWORD";
   const char* serverUrl = "http://192.168.1.100:8000/api/v1/camera/upload";
   ```

4. **Upload & Monitor**:
   - Connect **GPIO 0 to GND**, press reset button, and click **Upload**.
   - After upload succeeds, disconnect **GPIO 0 from GND**, open Serial Monitor at **115200 baud**, and press reset.

---

## 🌐 Server Integration

The ESP32-CAM posts image frames to `http://<SERVER_IP>:8000/api/v1/camera/upload`.

View live camera video stream in browser at:
`http://localhost:8000/live`
