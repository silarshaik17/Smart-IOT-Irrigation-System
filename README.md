# 🌱 Smart IoT Irrigation System

Automated, sensor-driven irrigation built on the **LPC2148 (ARM7)** microcontroller — reads live environmental data, makes an on-device decision about watering, and streams everything to the cloud in real time.

![Status](https://img.shields.io/badge/status-active-brightgreen)
![Platform](https://img.shields.io/badge/MCU-LPC2148-blue)
![Cloud](https://img.shields.io/badge/Cloud-ThingSpeak-orange)
![Language](https://img.shields.io/badge/language-Embedded%20C-blueviolet)

---

## 📖 Overview

Manual irrigation wastes water and guesses at soil conditions instead of measuring them. This project closes the loop: soil moisture and ambient temperature/humidity are sensed continuously, the microcontroller decides when — and for how long — to run the pump, and every action is logged to **ThingSpeak** so the system can be monitored remotely from a laptop or phone.


## 🧩 Block Diagram

<img src="images/modern_img.png" width="80%">

| Component | Role |
|---|---|
| LPC2148 (ARM7) | Central controller — sensor reads, decision logic, I/O |
| DHT11 | Temperature & humidity sensing |
| Soil Moisture Sensor | Primary irrigation trigger (digital/analog) |
| RTC | Timekeeping for scheduled cloud uploads |
| ESP01 | Wi-Fi bridge to ThingSpeak (UART) |
| 16x2 LCD | Local real-time display |
| 4x4 Keypad + Switch | Manual RTC configuration via interrupt |
| SPDT Relay + 12V Supply | Pump switching (LED substitute for demo) |

## 🛠️ Hardware Requirements

- LPC2148 development board
- DHT11 temperature & humidity sensor
- 16x2 LCD
- ESP01 Wi-Fi module
- Soil moisture sensor
- DB-9 cable / USB-UART converter
- SPDT relay, 12V supply, water pump (or LED for demo)

 
## Hardware Connections

| Component                           | LPC2148 Pin   |
| ----------------------------------- | ------------- |
| LCD Data Pins (D0–D7)               | P0.8 – P0.15  |
| 4×4 Keypad                          | P1.16 – P1.23 |
| Soil Moisture Sensor                | P0.21         |
| Water Pump/Motor (via Relay Driver) | P0.20         |
| DHT11 Temperature & Humidity Sensor | P0.23         |
| Interrupt Switch                    | P0.3 (EINT1)  |
| UART TX                             | P0.1 (TXD0)   |
| UART RX                             | P0.0 (RXD0)   |

## Overview of Hardware
<img src="images/overview.jpeg" width="75%">
## 💻 Software Requirements

- Keil µVision (C Compiler)
- Flash Magic (firmware flashing)
- Embedded C
- ThingSpeak account (free tier)

## ⚙️ How It Works

1. Power ON the system. The LPC2148 initializes all peripherals.
2. The LCD turns ON and displays the project name or initialization message.

   <img src="images/IMG_20260806_153258295_HDR.jpg" width="70%">

3. The ESP01 Wi-Fi module is initialized by sending AT commands (AT, ATE0, AT+CIPMUX, AT+CWQAP, etc.)

   <img src="images/Gallery_1786013512929.png" width="70%">
   <img src="images/Gallery_1786013578106.png" width="70%">
   <img src="images/Gallery_1786013603081.png" width="70%">
   <img src="images/Gallery_1786013578106.png" width="70%">
   <img src="images/Gallery_1786013853969.png" width="70%">
   <img src="images/Gallery_1786013578106.png" width="70%">
   <img src="images/Gallery_1786013701661.png" width="70%">
   <img src="images/Gallery_1786013869685.png" width="70%">

4. After the ESP01 successfully connects to the Wi-Fi network, the system starts normal operation.
5. The RTC reads the current date and time and displays it on the LCD.
6. The DHT11 sensor measures the temperature and humidity.
7. The image representing it shows on the LCD.

   <img src="images/Gallery_1785997738144.jpg" width="70%">

8. The soil moisture sensor measures the moisture level of the soil.
9. The LPC2148 displays the sensor values and time on the LCD.
10. If the soil moisture value is below the preset threshold, the LPC2148 turns ON the relay, which starts the 12 V water motor.
11. LCD Display

    <img src="images/Gallery_1786075843700.jpg" width="70%">

12. When the soil moisture reaches the required level, the relay turns OFF the water motor automatically.
13. The sensor data (temperature, humidity, soil moisture, and motor status) is sent to the ThingSpeak Cloud through the ESP01 using UART communication.
14. The uploaded data can be monitored from a mobile phone, laptop, or PC using the ThingSpeak dashboard.

**Execution sequence:**
Power ON → LCD Initialization → ESP01 AT Commands → Wi-Fi Connection → RTC → Sensor Reading → LCD Display → Motor Control → ThingSpeak Data Upload.

## 📊 Project Statistics
 Temperature Readings updated to cloud
<img src="images/Gallery_1786085942844.png" width="70%">

 Humidity Readings updated to cloud

<img src="images/Gallery_1786085906335.png" width="70%">

 Motor Readings updated to  cloud 
<img src="images/Gallery_1786085921844.png" width="70%">


- Sensor Data
- Temperature
- Soil Moisture

## 🔭 Future Scope

- Replace ESP01 with ESP32 for combined sensing + Wi-Fi on one chip
- Add a mobile app / dashboard for manual override
- Multi-zone irrigation with per-zone moisture thresholds
- Solar-powered standalone field deployment
