# Setup and Installation Guide

## 1. Install Arduino IDE

Download Arduino IDE 2.x from arduino.cc/en/software.

## 2. Add ESP32 Board Support

1. Open Arduino IDE
2. Go to File → Preferences
3. Paste this URL into "Additional boards manager URLs":
   `https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json`
4. Go to Tools → Board → Boards Manager
5. Search "esp32" and install the package by Espressif Systems

## 3. Install Required Libraries

Go to Sketch → Include Library → Manage Libraries and install:
- `ESP8266Audio` by Earle F. Philhower

The BLE libraries come bundled with the ESP32 board package.

## 4. Wire the Hardware

See the pin assignment table in [hardware/PARTS_LIST.md](../hardware/PARTS_LIST.md).

> Use a data-capable USB cable. Charge-only cables will not
> allow the IDE to detect your board.

## 5. Prepare the SD Card

1. Format the microSD card as FAT32
2. Copy your MP3 file to the root of the card
3. Rename it exactly: `Welcome.mp3`
4. Insert the card into the module before powering on

## 6. Upload the Code

1. Select your board: Tools → Board → ESP32S3 Dev Module
2. Select your port: Tools → Port → (the one that appeared when you plugged in)
3. Click Upload (the right arrow button)
4. Open Serial Monitor at 115200 baud and watch the startup output

## 7. Connect Your Phone (Stage 3 BLE only)

1. Install LightBlue (free on iOS and Android)
2. Scan for `ESP32_Receptionist`
3. Connect and subscribe to characteristic UUID:
   `beb5483e-36e1-4688-b7f5-ea07361b26a8`

## Troubleshooting

* Port not visible: Most likely due to bad USB cable connectiong. Try another cable. Disconnect for 5 seconds, then reconnect. And hit the reset button on the ESP32. It's the tiny button next to the ports, the one that says EN/RST.
* SD card mount failed: most likely culprit is card format or loose wiring. Fix: Reformat as FAT32; make sure connections are on the pins are 38/39/40.
* No audio: most likely due to wrong pin connection or the file wasn't found, sometimes, the culprit is low volume on the original audio.  Confirm Welcome.mp3 is in root; check pin 14; check the original file.
* Random triggers all the time:Threshold might be too low. Try increasing `movementThreshold` to 35.0 in the code. Play around with it to see which value works best for your environment.
* BLE not visible: Most likely Board not advertising. Check Serial Monitor for "BLE ready" message.