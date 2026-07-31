# 🌱 Smart IoT Irrigation System

Automated, sensor-driven irrigation built on the **LPC2148 (ARM7)** microcontroller — reads live environmental data, makes an on-device decision about watering, and streams everything to the cloud in real time.

![Status](https://img.shields.io/badge/status-active-brightgreen)
![Platform](https://img.shields.io/badge/MCU-LPC2148-blue)
![Cloud](https://img.shields.io/badge/Cloud-ThingSpeak-orange)
![Language](https://img.shields.io/badge/language-Embedded%20C-blueviolet)

---

## 📖 Overview

Manual irrigation wastes water and guesses at soil conditions instead of measuring them. This project closes the loop: soil moisture and ambient temperature/humidity are sensed continuously, the microcontroller decides when — and for how long — to run the pump, and every action is logged to **ThingSpeak** so the system can be monitored remotely from a laptop or phone.

## ⚙️ How It Works

1. **DHT11** reads temperature and relative humidity; values are displayed on a 16x2 LCD in real time.
2. **Soil moisture sensor** continuously checks soil condition (digital or analog output).
3. **Decision logic** on the LPC2148:
   - Soil moisture is the **primary** trigger — if it's low, the pump turns on.
   - Temperature acts as a **modifier**:
     - Dry soil + high temperature → pump runs **3 minutes**
     - Dry soil + normal/low temperature → pump runs **1 minute**
   - Soil moist → pump stays off.
4. **RTC** timestamps sensor readings and gates how frequently data is pushed to the cloud.
5. **ESP01 Wi-Fi module** (driven over UART) pushes temperature, humidity, and pump ON/OFF events to **ThingSpeak**, viewable live from any laptop/PC/mobile.
6. A **4x4 matrix keypad + interrupt switch** let the user manually edit RTC time via a menu-driven interface.

> 💡 A LED is used in place of the actual water pump/motor for safe demonstration — swap in a relay + 12V pump for a real deployment.

## 🧩 System Architecture

![Block Diagram](docs/block_diagram.png)

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

## 💻 Software Requirements

- Keil µVision (C Compiler)
- Flash Magic (firmware flashing)
- Embedded C
- ThingSpeak account (free tier)

## 🚀 Getting Started

```bash
git clone https://github.com/<your-username>/smart-iot-irrigation-system.git
```

1. Open the project in **Keil µVision**.
2. Wire up hardware per `docs/circuit_diagram.png`.
3. Set your ThingSpeak **Write API Key** in `esp01.h`.
4. Build and flash to the LPC2148 using **Flash Magic**.
5. Watch live data at `https://thingspeak.com/channels/<your-channel-id>`.

## 📊 Results

*(Add a screenshot of your ThingSpeak dashboard here, and a short demo GIF/video of the LCD + pump reacting to soil moisture changes.)*

```
docs/thingspeak_dashboard.png
media/demo.gif
```

## 🔭 Future Scope

- Replace ESP01 with ESP32 for combined sensing + Wi-Fi on one chip
- Add a mobile app / dashboard for manual override
- Multi-zone irrigation with per-zone moisture thresholds
- Solar-powered standalone field deployment

## 📄 License

MIT License — free to use and modify for learning and non-commercial projects.

---

*Built as part of an embedded systems / IoT coursework project.*
