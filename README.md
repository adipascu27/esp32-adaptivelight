# 🌙 Adaptive IoT Terrace Lighting System

An IoT-based adaptive lighting system designed for a home terrace. The system combines an **ESP32**, ambient light and motion sensors, addressable LEDs, **Firebase Realtime Database**, and a **React web application** to provide both remote and automatic control of the lighting.

The project was developed as my bachelor's thesis and includes both the hardware implementation and the software infrastructure required to control and monitor the system remotely.

---

## ✨ Features

- 💡 Remote LED control through a web interface
- 🎨 RGB color control for the LED strip
- 🔆 Adjustable brightness
- 🌙 Automatic lighting based on ambient light
- 🚶 Motion detection using a PIR sensor
- ☁️ Real-time communication through Firebase
- 🔐 Firebase Authentication and database security rules
- 📊 Live sensor status displayed in the web interface
- 🔄 Automatic ESP32 reconnection after network interruptions
- 🌓 Visual indication of the current light threshold
- ⏱️ Automatic lighting duration after motion detection

---

# 🏗️ System Architecture

The system is divided into three main components:

```text
                    ┌─────────────────────┐
                    │     React Web App   │
                    │                     │
                    │  • Manual Control   │
                    │  • RGB / Brightness │
                    │  • Sensor Status    │
                    └──────────┬──────────┘
                               │
                               │ Real-time
                               │ communication
                               ▼
                    ┌─────────────────────┐
                    │ Firebase Realtime   │
                    │      Database       │
                    │                     │
                    │ • LED state         │
                    │ • Brightness        │
                    │ • RGB values        │
                    │ • Operating mode    │
                    │ • Sensor data       │
                    └──────────┬──────────┘
                               │
                               │ Wi-Fi
                               ▼
                    ┌─────────────────────┐
                    │        ESP32        │
                    │                     │
                    │ • Control logic     │
                    │ • Sensor reading    │
                    │ • LED control       │
                    └──────┬───────┬──────┘
                           │       │
                 ┌─────────┘       └─────────┐
                 ▼                           ▼
        ┌─────────────────┐         ┌─────────────────┐
        │   BH1750FVI     │         │    HC-SR501     │
        │  Light Sensor   │         │   PIR Sensor    │
        └─────────────────┘         └─────────────────┘

                           │
                           ▼
                  ┌──────────────────┐
                  │    WS2812B LED   │
                  │      Strip       │
                  └──────────────────┘