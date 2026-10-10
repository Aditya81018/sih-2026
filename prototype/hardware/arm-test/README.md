# ESP32 4-Servo Robotic Arm Controller (PCA9685 I2C)

This directory contains the ESP32 firmware for real-time 4-servo control over WebSockets using an **Adafruit PCA9685 16-Channel 12-bit PWM Servo Driver** via I2C.

---

## 🛠️ Hardware Requirements

1. **ESP32 Development Board** (e.g., ESP32 DevKit V1, NodeMCU-32S, or AI-Thinker ESP32-CAM).
2. **PCA9685 16-Channel PWM Servo Driver Module** (I2C address `0x40`).
3. **4x Standard RC Servos** (e.g., SG90, MG995, or MG996R) connected to Channels `0`, `1`, `2`, and `3` on the PCA9685 module.
4. **External 5V DC Power Supply** connected to the PCA9685 V+ screw terminal for powering the servos.

---

## 📌 Pinout & I2C Wiring

| Component | PCA9685 Channel / ESP32 Pin | Notes |
| :--- | :--- | :--- |
| **Servo 1 (Base / Joint 1)** | Channel `0` | PCA9685 PWM Output 0 |
| **Servo 2 (Arm / Joint 2)** | Channel `1` | PCA9685 PWM Output 1 |
| **Servo 3 (Wrist / Joint 3)** | Channel `2` | PCA9685 PWM Output 2 |
| **Servo 4 (Gripper / End Effector)** | Channel `3` | PCA9685 PWM Output 3 |
| **SDA** | `GPIO 21` | I2C Data Pin |
| **SCL** | `GPIO 22` | I2C Clock Pin |
| **VCC** | `3.3V` / `5V` | Logic power for PCA9685 chip |
| **GND** | `GND` | Common Ground with ESP32 & Power Supply |
| **V+ (Terminal)** | External `5V` | Servo power supply (5V - 6V DC) |

---

## 💻 Software & Library Setup (Arduino IDE)

1. Open **Arduino IDE**.
2. Go to **Tools -> Board -> ESP32 Arduino** and select **ESP32 Dev Module**.
3. Open **Tools -> Manage Libraries...** (`Ctrl + Shift + I`):
   - Search for **Adafruit PWM Servo Driver Library** and click **Install**.
   - Search for **ArduinoJson** by Benoit Blanchon and click **Install** (v6 or v7).
4. Update Wi-Fi settings in `arm-test.ino` if needed:
   ```cpp
   const char *ssid = "iQOO_Z10_X";
   const char *password = "iQooZ10x";
   const char *serverHost = "10.226.94.110";
   ```
5. Click **Upload** to flash the code to your ESP32.

---

## 🚀 Web Controller Dashboard

Start the FastAPI server and open the arm control page in your web browser:

- **Arm Control Web UI**: `http://10.226.94.110:8000/arm`
- **Camera Live Stream UI**: `http://10.226.94.110:8000/live`

Moving any of the 4 sliders (Servo 1, Servo 2, Servo 3, or Servo 4) on the `/arm` webpage immediately sends JSON WebSocket frames (`{"servo1": a1, "servo2": a2, "servo3": a3, "servo4": a4}`) to the server, which forwards them in real-time to the ESP32 to drive Channels 0, 1, 2, and 3!
