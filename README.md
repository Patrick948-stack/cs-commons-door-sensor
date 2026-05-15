# CS Commons Door Sensor

An ESP32-based smart door sensor built for a college CS commons room.  
The system measures the doorway distance at the beginning with the doorway empty.
This becomes the baseline. Every 300 milliseconds, it takes a new averaged reading.
If the difference exceeds 20 cm, movement is detected. A 5-second waiting period
prevents repeated triggers from a single entry.

![CS Commons Door Sensor Project Architecture](https://raw.githubusercontent.com/Patrick948-stack/cs-commons-door-sensor/main/hardware/images/diagram.png) 
*A Diagram showing the logic flow and architecture of the system (Image generated with GPT Image 2 after careful, specific prompting to ensure accuracy to the actual project logic)*



## Project Stages

This repository is organized by development stage. Each stage has its own branch so you can see exactly how the project evolved.

| Stage | Branch | Description | Status |
|-------|--------|-------------|--------|
| 1 | `stage-1-basic-sensor` | Ultrasonic detection + MP3 playback | Complete |
| 2 | `stage-2-ble` | BLE phone notifications added | Complete |
| 3 | `stage-3-camera` | Camera web server integration | Not yet started, for future development |


## Hardware

| Component | Purpose |
|-----------|---------|
| ESP32-S3 Dev Board | Main microcontroller |
| HC-SR04 Ultrasonic Sensor | Measures doorway distance |
| MicroSD Card Module | Stores the MP3 audio file |
| Small Speaker | Audio output (via I2S) |
| MicroSD Card (FAT32) | Holds `Welcome.mp3` |


## Stage 1 — Basic Door Sensor

### What It Does
At this stage, our system measure the baseline distance (distance from sensor to 
the empty doorway), continuously monitors changes in distance to detect a potential
visitor between the doorway and the sensor. When the sensor detects a change in 
distance greater than 20cm. When the signal is detected, the system plays an MP3 
file with a message saying "Welcome to the CS Lounge". We set the 20 cm distance to 
reduce noise detection where the system mistakes small fluctuations in the environment 
for visitors.


### Wiring

![CS Commons Door Sensor Wiring](https://raw.githubusercontent.com/Patrick948-stack/cs-commons-door-sensor/main/hardware/images/circuit-diagram.png)
*A Diagram showing the full wiring architecture (Image generated with GPT Image 2 after careful, specific prompting to ensure accuracy to the actual project wiring)*


| Sensor / Module | ESP32 Pin |
|-----------------|-----------|
| Ultrasonic TRIG | GPIO 11 |
| Ultrasonic ECHO | GPIO 12 |
| SD MMC CMD      | GPIO 38 |
| SD MMC CLK      | GPIO 39 |
| SD MMC D0       | GPIO 40 |
| Audio BCLK      | GPIO 16 |
| Audio LRC       | GPIO 17 |
| Audio DOUT      | GPIO 14 |

## Software Setup

### Requirements
- Arduino IDE 2.x
- ESP32 board package by Espressif (installed via Boards Manager)
- [ESP8266Audio](https://github.com/earlephilhower/ESP8266Audio) library

### Installation

1. Clone this repository
2. Open the `.ino` file for your desired stage in Arduino IDE
3. Select your board: **Tools → Board → ESP32S3 Dev Module**
4. Select your port: **Tools → Port → [your port]**
5. Upload

### Key Settings You Can Play Around With

```cpp
const float movementThreshold = 20.0;  // cm — increase if false triggers happen
const unsigned long cooldownTime = 5000; // ms — time before it can trigger again, increase if you're getting notifications too many times
```

### How To Use

1. Format your microSD card as FAT32
2. Copy `Welcome.mp3` to the root of the card
3. Wire the hardware as shown above
4. Install the Arduino IDE and add ESP32 board support
5. Install the `ESP8266Audio` library via Library Manager
6. Open `stage-1-basic-sensor/door_sensor.ino`
7. Select your board under **Tools → Board → ESP32S3 Dev Module**
8. Click Upload
9. Open Serial Monitor at **115200 baud** and watch the output

### What You Should See in Serial Monitor

Starting CS Commons Door Sensor...
SD card initialized.
Found /Welcome.mp3 | Size: 142080 bytes
Calibrating doorway distance...
Base doorway distance: XYZ cm
System ready.
Current distance: ABC cm | Difference: D
Current distance: EFG cm | Difference: G
Movement detected. Playing welcome message.


## Repository Structure

cs-commons-door-sensor/
├── firmware/          # Arduino source code, one folder for each development stage stage
├── hardware/          # Parts list, wiring diagrams, photos
├── docs/              # Setup guide and troubleshooting
├── README.md
└── LICENSE

## Authors

- **Arvin Shirchindorj** — original ultrasonic + audio system
- **Ali Abbaka** — original ultrasonic + audio system  
- **Patrick Mulikuza** — BLE integration, code documentation, repository


## License

MIT License — see [LICENSE](LICENSE) for details.
