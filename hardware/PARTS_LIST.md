# Parts List

## Core Components

| Component | Quantity | Purpose | Notes |
|-----------|----------|---------|-------|
| ESP32-S3 Dev Board | 1 | Main microcontroller | Any ESP32-S3 variant works |
| HC-SR04 Ultrasonic Sensor | 1 | Measures doorway distance | 5V powered |
| MicroSD Card Module | 1 | Stores the MP3 audio file | SPI or SDMMC interface |
| MicroSD Card (8GB or less) | 1 | FAT32 formatted | Larger cards sometimes fail |
| Small 8Ω Speaker | 1 | Audio output | Paired with a small amplifier or direct I2S |
| Jumper Wires (M-M) | ~20 | Connections | Assorted lengths |
| Breadboard | 1 | Prototyping | Half-size is fine |
| USB-C Cable (data-capable) | 1 | Programming and power | Must carry data, not charge-only |

## Pin Assignments

| GPIO Pin | Connected To | Direction |
|----------|-------------|-----------|
| 11 | HC-SR04 TRIG | Output |
| 12 | HC-SR04 ECHO | Input |
| 14 | Speaker (I2S Data) | Output |
| 16 | I2S BCLK | Output |
| 17 | I2S LRC | Output |
| 38 | SD Card CMD | Output |
| 39 | SD Card CLK | Output |
| 40 | SD Card D0 | Input/Output |

## Audio File

Place a file named exactly `Welcome.mp3` in the root folder of the
microSD card before inserting it. The filename is case-sensitive.