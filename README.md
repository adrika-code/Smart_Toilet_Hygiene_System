# Smart Public Toilet Hygiene Monitoring System

An IoT system built on ESP32 that monitors air quality, water level, and dustbin/tank fill level in a public toilet — with automatic buzzer alerts and a live web dashboard hosted directly on the ESP32 (no cloud or internet needed).

## Features
- Real-time gas/air quality monitoring (MQ135)
- Bin/tank fill-level detection (HC-SR04 ultrasonic)
- Water level monitoring
- Automatic buzzer alert when any reading crosses a set threshold
- Live web dashboard hosted directly on the ESP32, accessible from any device on the same WiFi network — dark-themed control-panel style UI with live circular gauges for each sensor and a status banner that turns red when attention is needed

## Hardware Used
| Component | Purpose |
|---|---|
| ESP32 Dev Board | Main microcontroller + WiFi |
| MQ135 Gas Sensor | Air quality / odor detection |
| HC-SR04 Ultrasonic Sensor | Bin/tank fill level |
| Water Level Sensor | Water level detection |
| Buzzer | Audible alert |

## Wiring / Pin Map
| Component | ESP32 Pin | Notes |
|---|---|---|
| MQ135 (Analog Out) | GPIO34 | Needs 5V on VCC |
| Water Sensor (Signal) | GPIO32 | |
| HC-SR04 TRIG | GPIO5 | |
| HC-SR04 ECHO | GPIO18 | Needs 5V on VCC; ECHO is a 5V signal into a 3.3V-rated GPIO — fine for short-term use, add a voltage divider for long-term durability |
| Buzzer (I/O) | GPIO23 | |

All GND pins share a common ground rail with the ESP32. All VCC pins share a common power rail (check each sensor's required voltage individually — noted above where it matters).

## How It Works
1. On boot, the ESP32 connects to WiFi and starts a local web server.
2. Every 2 seconds, it reads all three sensors and evaluates them against fixed thresholds (adjustable in code):
   - Gas value > 1800 → air quality alert
   - Water value < 500 → low water alert
   - Bin distance < 10cm → bin considered full
3. If any condition is true, the buzzer turns on and the dashboard's status banner switches to red.
4. The dashboard (served at the ESP32's IP address) polls `/data` every 2 seconds and updates the gauges and status live, no page refresh needed.

## Setup Instructions
1. Install the ESP32 board package in Arduino IDE (Boards Manager → search "esp32").
2. Open `smart_toilet_hygiene.ino` in Arduino IDE.
3. Enter your WiFi credentials near the top of the file:
   ```cpp
   const char* ssid     = "YOUR_WIFI_NAME";
   const char* password = "YOUR_WIFI_PASSWORD";
   ```
4. Adjust threshold values if needed for your environment (`GAS_THRESHOLD`, `WATER_LOW`, `BIN_FULL_CM`).
5. Select **ESP32 Dev Module** as the board, select the correct COM port, and upload.
6. Open Serial Monitor at **115200 baud** — once connected, it prints the device's IP address.
7. Open that IP address in a browser on the same WiFi network to view the live dashboard.

## Future Improvements
- Add DHT11 for temperature/humidity tracking
- Cloud logging (e.g. Firebase) for historical data and remote access
- LCD for on-device status display
- SMS/email alerts for maintenance staff
- Voltage divider on the HC-SR04 ECHO line for long-term GPIO protection
