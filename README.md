# ResQBot

# 🚨 ResQBot: Edge IoT Disaster Response Rover
🏆 **2nd Place Winner - Hardware Problem Statement** ResQBot is an autonomous, infrastructure-independent disaster management rover designed for zero-connectivity environments. Built on the ESP32 platform, it acts as a localized Edge IoT web server, providing first responders with live mapping, acoustic survivor detection, and terrain telemetry without requiring standard internet access.

## ✨ Core Features
* **Edge IoT Dashboard:** Broadcasts a local "SoftAP" Wi-Fi network and hosts an interactive HTML dashboard accessible from any rescuer's smartphone.
* **Autonomous Evasion:** Fuses MPU6050 gyroscope data with HC-SR04 ultrasonic sensors to navigate disaster rubble and auto-correct steering.
* **Dynamic Torque Boost:** Automatically detects rough terrain (tilt > 3.0) and overrides base motor speed to climb over obstacles.
* **Survivor Detection:** Integrates acoustic sensors to flag potential survivor locations on the live dashboard.
* **Live GPS Mapping:** Uses a NEO-6M module to provide exact coordinate tracking for rescue teams.

## 🛠️ Hardware Bill of Materials (BOM)
* ESP32 Development Board (Microcontroller & Web Server)
* L298N Motor Driver
* 4x DC Gear Motors & Wheels
* MPU6050 (6-Axis Gyroscope/Accelerometer)
* HC-SR04 (Ultrasonic Sensor)
* Sound Detection Sensor (Digital)
* NEO-6M GPS Module
* 11.1V 3S LiPo Battery (Recommended for maximum torque)

## ⚡ Circuit Pinout
| Component | Pin | ESP32 Pin / Connection |
| :--- | :--- | :--- |
| **L298N** | ENA / ENB | GPIO 25 / GPIO 13 |
| **L298N** | IN1 / IN2 | GPIO 26 / GPIO 27 |
| **L298N** | IN3 / IN4 | GPIO 14 / GPIO 5 |
| **MPU6050** | SDA / SCL | GPIO 21 / GPIO 22 |
| **Ultrasonic**| TRIG / ECHO | GPIO 4 / GPIO 39 |
| **Sound** | OUT | GPIO 35 |
| **GPS** | TX / RX | GPIO 16 / GPIO 17 |

*(Note: The ESP32 is powered via the 5V output of the L298N to prevent voltage drops during motor spikes).*

## 🚀 Setup & Installation
1. Install the required libraries in the Arduino IDE: `WiFi`, `WebServer`, `Wire`, `Adafruit_MPU6050`, `TinyGPSPlus`.
2. Connect the hardware according to the pinout table above. **Crucial:** Keep ESP32 Pin 12 completely disconnected during code upload to prevent strapping pin crashes.
3. Flash `ResQBot_Final.ino` to the ESP32 using a baud rate of `115200`.
4. Power the system with the 12V battery.
5. On your smartphone, connect to the Wi-Fi network: **`ResQBot_Emergency`** (Password: `rescue1234`).
6. Open a web browser and navigate to **`192.168.4.1`** to view the live dashboard.

## 👨‍💻 Team Mech-X
This project was developed collaboratively by:
* **John Stewartsson J R** - Hardware Engineering & Systems Integration
* **[Friend 1 Name]** - [Their Role, e.g., Embedded C++ Developer]
* **[Friend 2 Name]** - [Their Role, e.g., IoT & Network Config]
* **[Friend 3 Name]** - [Their Role, e.g., Sensor Calibration]
* **[Friend 4 Name]** - [Their Role, e.g., Mechanical Assembly]

## 📄 License
This project is licensed under the MIT License - see the LICENSE file for details.
