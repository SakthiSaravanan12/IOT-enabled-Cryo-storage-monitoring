# ❄️ Cryo Guardian — IoT Enabled Cryo Storage Monitoring and Alert System

A real-time IoT monitoring system built with **ESP32** that tracks temperature and liquid nitrogen (LN2) levels inside cryo storage units — with live TFT display, Telegram bot alerts, and a web dashboard.

---

## 📌 Project Overview

Cryo storage units must maintain extremely low temperatures (around -196°C) to safely preserve biological samples. Any rise in temperature or drop in LN2 level can cause irreversible damage. **Cryo Guardian** continuously monitors these parameters and instantly alerts the operator via Telegram when critical thresholds are crossed.

---

## ✨ Key Features

- 🌡️ **Real-time temperature monitoring** using MAX31865 RTD sensor
- 💧 **LN2 level detection** with digital sensor input
- 📱 **Telegram bot alerts** — instant notification when readings go critical
- 🖥️ **2.4-inch TFT display** with 5 rotating UI pages
- 🌐 **IoT Web Dashboard** — accessible from any browser on the same network
- 🔊 **Buzzer alarm** — continuous for critical, pulsing for warning
- 📋 **History logs** stored on LittleFS (ESP32 flash memory)
- ❤️ **Heartbeat indicator** on display to confirm system is live
- 🐕 **Watchdog timer** to auto-restart if system hangs

---

## 🖥️ Display Pages

| Page | Content |
|------|---------|
| 0 | Welcome Screen |
| 1 | Live Monitor — Temperature + LN2 Status + Stability % |
| 2 | Network Status — WiFi, IP Address, Uptime |
| 3 | Alarm Config — Safe range and current status |
| 4 | History Logs — Last 7 log entries |

---

## 🛠️ Tech Stack & Hardware

**Hardware:**
- ESP32 Microcontroller
- MAX31865 RTD Temperature Sensor (PT100)
- 2.4-inch TFT Display (ST7789 driver)
- LN2 Level Sensor (digital input)
- Buzzer

**Software & Libraries:**
- Arduino IDE (C++)
- Adafruit GFX + ST7789 — TFT display
- Adafruit MAX31865 — temperature sensor
- UniversalTelegramBot — Telegram alerts
- LittleFS — flash file storage
- WebServer — IoT browser dashboard
- ArduinoJson — JSON parsing

---

## 📁 Project Structure

```
cryo-guardian/
│
├── cryo_guardian.ino    # Main Arduino sketch
└── README.md
```

---

## ⚙️ How to Set Up

**1. Install Arduino Libraries**

In Arduino IDE → Library Manager, install:
- `Adafruit GFX Library`
- `Adafruit ST7789`
- `Adafruit MAX31865`
- `UniversalTelegramBot`
- `ArduinoJson`

**2. Configure your credentials**

Open `cryo_guardian.ino` and update:
```cpp
const char* WIFI_SSID = "YOUR_WIFI_NAME";
const char* WIFI_PASS = "YOUR_WIFI_PASSWORD";
#define BOTtoken "YOUR_TELEGRAM_BOT_TOKEN"
#define CHAT_ID  "YOUR_TELEGRAM_CHAT_ID"
```

**3. Pin Connections**

| Component | ESP32 Pin |
|-----------|-----------|
| TFT CS | GPIO 5 |
| TFT DC | GPIO 2 |
| TFT RST | GPIO 4 |
| MAX31865 CS | GPIO 15 |
| LN2 Sensor | GPIO 14 |
| Buzzer | GPIO 25 |
| SPI CLK | GPIO 18 |
| SPI MISO | GPIO 19 |
| SPI MOSI | GPIO 23 |

**4. Upload & Run**

Select **ESP32 Dev Module** in Arduino IDE, upload the sketch, and open Serial Monitor at 115200 baud.

---

## 🌐 Web Dashboard

Once connected to WiFi, open a browser and go to:
```
http://<ESP32_IP_ADDRESS>
```
The dashboard auto-refreshes every 5 seconds showing live temperature, LN2 level, and system status.

---

## ⚠️ Alert Thresholds

| Condition | Action |
|-----------|--------|
| Temperature > -150°C | Warning / Critical alarm |
| LN2 Level LOW | Warning / Critical alarm |
| Both critical | Continuous buzzer + Telegram alert |

---

## 👤 Author

**D Sakthi Saravanan**
Final Year B.E Computer Science Engineering
Saveetha Institute of Medical and Technical Sciences

---

## 📄 License

This project is for academic and research purposes.
