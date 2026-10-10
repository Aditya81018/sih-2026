# ESP32 275° Sonar Radar / LIDAR Test (`lidar-test`)

This module provides hardware firmware and documentation to control a **28BYJ-48 Stepper Motor** with a **ULN2003 Driver Board** and an **HC-SR04 Sonar (Ultrasonic Distance Sensor)** mounted on the motor's shaft.

It sweeps **275° continuously** (0° -> 275° clockwise, then 275° -> 0° counter-clockwise) in **1° increments**, measuring distance at every step and streaming live telemetry to the FastAPI server via WebSockets.

---

## 📷 Hardware Setup & Components

1. **ESP32 Microcontroller Board** (DevKit V1 / AI-Thinker / NodeMCU 32S)
2. **28BYJ-48 5V Unipolar Stepper Motor** (4-phase, 1:64 gear reduction ratio)
3. **ULN2003 Stepper Motor Driver Board**
4. **HC-SR04 Ultrasonic Distance Sensor**
5. **5V Power Supply** (Recommended external 5V 2A power source for ULN2003 & Sonar)

---

## 🔌 Pin Wiring Table

| Component | Component Pin | ESP32 GPIO Pin | Wire Color / Note |
|---|---|---|---|
| **ULN2003 Driver** | `IN1` | `GPIO 19` | Control Coil A |
| **ULN2003 Driver** | `IN2` | `GPIO 18` | Control Coil B |
| **ULN2003 Driver** | `IN3` | `GPIO 5` | Control Coil C |
| **ULN2003 Driver** | `IN4` | `GPIO 17` | Control Coil D |
| **ULN2003 Driver** | `+` (5-12V) | External `5V` VCC | Red Power Terminal |
| **ULN2003 Driver** | `-` (GND) | `GND` | Common Ground with ESP32 |
| **HC-SR04 Sonar** | `VCC` | `5V` (or Vin) | Sensor Power (5V) |
| **HC-SR04 Sonar** | `GND` | `GND` | Common Ground |
| **HC-SR04 Sonar** | `TRIG` | `GPIO 23` | Pulse Trigger Input |
| **HC-SR04 Sonar** | `ECHO` | `GPIO 22` | Echo Pulse Output |

> ⚠️ **Important Grounding Note**: Always connect the GND of your external 5V power supply, ESP32 GND, and ULN2003 GND together.

---

## 📐 Stepper Motor Mathematics (1° Precision)

- The **28BYJ-48 5V Stepper Motor** uses an internal 64:1 reduction gearbox.
- Using 8-step half-stepping (sequence: `A`, `AB`, `B`, `BC`, `C`, `CD`, `D`, `DA`), 1 full output rotation (360°) requires **4096 half-steps**.
- Steps per degree calculation:
  $$\text{Steps per Degree} = \frac{4096}{360} \approx 11.3777778 \text{ steps/deg}$$
- Total half-steps for $275^\circ$:
  $$\text{Total Steps} = \text{round}(275 \times 11.3777778) = 3129 \text{ steps}$$
- Zero-drift guarantee: The code uses absolute coordinate tracking `round(targetAngle * STEPS_PER_DEGREE)` to eliminate step accumulation errors over long continuous sweeps.

---

## 🚀 How to Flash & Run

1. Open `hardware/lidar-test/lidar-test.ino` in **Arduino IDE**.
2. Select Board: `Tools` > `Board` > `ESP32 Arduino` > `ESP32 Dev Module`.
3. Ensure required libraries are installed:
   - `ArduinoJson` (v6 or v7) by Benoit Blanchon
   - `WiFi` (built-in for ESP32)
4. Update Wi-Fi SSID, Password, and Server IP address in the code:
   ```cpp
   const char *ssid = "YOUR_WIFI_SSID";
   const char *password = "YOUR_WIFI_PASSWORD";
   const char *serverHost = "192.168.X.X"; // Your FastAPI Server IP
   ```
5. Click **Upload** and open **Serial Monitor** at `115200` baud.
6. Open your browser and navigate to `http://<SERVER_IP>:8000/lidar` to view the live radar scan map!

---

## 🛰️ WebSocket Data Protocol

### ESP32 Telemetry Packet (Sent to `/api/v1/lidar/ws/device`)
```json
{
  "angle": 142,
  "distance": 48.5,
  "sweep_direction": "CW",
  "timestamp": 128450
}
```
*Note: If no obstacle is detected within 400cm, `distance` is `null`.*

### Web Controller Command Packet (Sent to `/api/v1/lidar/ws/controller`)
```json
{
  "active": true,
  "reset": false
}
```
