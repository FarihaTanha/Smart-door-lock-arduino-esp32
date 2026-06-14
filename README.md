# Smart-door-lock-arduino-esp32
# Smart Door Lock System — Arduino & ESP32-CAM

A multi-layer intelligent door lock system built with Arduino UNO and ESP32-CAM, supporting three independent authentication modes: keypad password, face recognition, and remote Telegram control.

---

## Overview

Traditional door locks rely on a single point of access — a key or a PIN. This project replaces that with a layered security system that combines physical input, computer vision, and IoT remote control into one embedded solution.

Built as a hardware-software integrated system for the CSE360: Computer Interfacing course at BRAC University.

---

## Features

- **Keypad Authentication** — 4-digit password entry via a 4x4 keypad
- **Face Recognition** — ESP32-CAM detects and verifies registered faces
- **Telegram Remote Control** — Owner can lock/unlock and receive photos remotely via a Telegram Bot
- **Security Alert** — On wrong password, ESP32-CAM captures and sends a photo to the owner via Telegram
- **Auto-Lock** — Hall effect sensor detects door closure and automatically re-locks
- **Real-time LCD Feedback** — 20x4 I2C LCD displays live system status

---

## Hardware Components

| Component | Quantity |
|---|---|
| Arduino UNO | 1 |
| ESP32-CAM Module | 2 |
| 4x4 Keypad | 1 |
| 20x4 I2C LCD Display | 1 |
| Micro Servo Motor | 1 |
| Hall Effect Sensor | 1 |
| BC547 Transistor | 1 |
| 10kΩ Resistor | 1 |
| Breadboard | 2 |
| Jumper Wires | — |
| Power Supply (3A + 2A) | 2 |

**Estimated Build Cost: ~7,000 BDT**

---

## Communication Protocols Used

| Protocol | Purpose |
|---|---|
| I2C | Arduino ↔ LCD |
| GPIO | ESP32-CAM ↔ Arduino signaling |
| PWM | Servo motor control |
| WiFi + HTTPS | ESP32-CAM ↔ Telegram Bot API |
| HTTP | ESP32-CAM web server |
| UART | Serial debugging |

---

## How It Works

### 1. Keypad Mode
User enters the 4-digit password. If correct → servo unlocks. If wrong → ESP32-CAM captures a photo and sends it to the owner via Telegram.

### 2. Face Recognition Mode
User stands in front of the ESP32-CAM. If the face matches stored data → GPIO HIGH signal sent to Arduino → door unlocks.

### 3. Telegram Mode
Owner sends commands remotely via Telegram Bot:
- `/photo` — Capture and send a photo
- `/unlock` — Unlock the door remotely
- `/lock` — Lock the door remotely
- `/start` — Initialize the bot

### Auto-Lock
Once the door is closed, the hall effect sensor detects the magnet, waits a preset duration, then automatically re-locks the servo.

---

## Project Structure

```
smart-door-lock-arduino-esp32/
│
├── arduino/
│   └── main.ino               # Arduino code (keypad, LCD, servo, hall sensor)
│
├── esp32cam/
│   ├── face_recognition.ino   # ESP32-CAM face recognition + GPIO signal
│   └── telegram_control.ino   # ESP32-CAM Telegram bot + photo capture
│
├── circuit/
│   └── schematic.png          # Full wiring diagram
│
├── docs/
│   └── report.pdf             # Full lab project report
│
└── README.md
```

---

## Setup & Installation

### Arduino
1. Install the Arduino IDE
2. Install libraries: `LiquidCrystal_I2C`, `Keypad`, `Servo`
3. Open `arduino/main.ino` and upload to Arduino UNO

### ESP32-CAM
1. In Arduino IDE, add ESP32 board support via Board Manager
2. Select board: `AI Thinker ESP32-CAM`
3. Install libraries: `UniversalTelegramBot`, `ArduinoJson`
4. Set your WiFi credentials and Telegram Bot token in the code
5. Upload `esp32cam/face_recognition.ino` and `esp32cam/telegram_control.ino` to respective modules

### Telegram Bot
1. Create a bot via [@BotFather](https://t.me/BotFather) on Telegram
2. Copy your bot token into the ESP32-CAM code
3. Get your Telegram chat ID and add it to the code

---

## Test Results

| Test Case | Status |
|---|---|
| Correct keypad password → unlock | ✅ Pass |
| Wrong password → photo sent to Telegram | ✅ Pass |
| Registered face → unlock | ✅ Pass |
| `/unlock` Telegram command | ✅ Pass |
| Auto-lock after door closes | ✅ Pass |
| Real-time LCD status updates | ✅ Pass |

---

## Known Limitations

- Face recognition accuracy drops in low-light environments
- System depends on a stable WiFi connection for Telegram features
- Hardware wiring complexity can make debugging time-consuming

---

## Future Improvements

- Add fingerprint sensor as a fourth authentication method
- Build a companion mobile app
- Improve face recognition model accuracy
- Add offline fallback mode when WiFi is unavailable

---

## Contributors

| Name | ID | Role |
|---|---|---|
| Fariha Tanha | 22101296 
| Shamaila Sadat Niha | 22301728 
| Pranto Roy | 22301261 
| Nishat Tasnim | 22301643 

## 📄 License

This project is open-source and available under the [MIT License](LICENSE).
