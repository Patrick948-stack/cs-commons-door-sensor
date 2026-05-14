/**********************************************************************
  Project: CS Commons Door Sensor + BLE Phone Notification
  Original: Arvin Shirchindorj, Ali Abbaka
  BLE addition: Patrick Mulikuza

  What this does:
  - Ultrasonic sensor measures doorway distance
  - When someone enters, plays Welcome.mp3
  - Then notifies a phone via Blootooth

  Skeleton of the project:
  Start 
  Measure distance to empty doorway 
  Loop forever:
    Measure distance
    Compare
    If big change:
        If cooldown passed:
            Play MP3
            Send BLE alert to phone
**********************************************************************/
/**********************************************************************
The libraries: 
* Arduino.h: used for basic operations for things like pins and timing
* SD_MMC.h: for handling the SD Card

**********************************************************************/


#include "Arduino.h"
#include "WiFi.h"
#include "FS.h"
#include "SD_MMC.h"
#include "AudioFileSourceSD_MMC.h"
#include "AudioFileSourceID3.h"
#include "AudioGeneratorMP3.h"
#include "AudioOutputI2SNoDAC.h"

#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>

/************************************************************
  SD card pins
************************************************************/
#define SD_MMC_CMD 38
#define SD_MMC_CLK 39
#define SD_MMC_D0  40
#define WELCOME_FILE "/Welcome.mp3"

/************************************************************
  Ultrasonic sensor pins
************************************************************/
#define trigPin 11      // for sending the signal
#define echoPin 12      // for hearing the echo
#define MAX_DISTANCE 200

float timeOut = MAX_DISTANCE * 60;
int soundVelocity = 340;

/************************************************************
  Door detection settings
************************************************************/
float baseDistance = 0;
const float movementThreshold = 40.0;              // initially set to 20, but changed to 40 since the previous value was too sensitive to noise
const unsigned long cooldownTime = 15000;         // The amount of time it should wait before taking other measurements. Without this,
                                                 // the system spammed us with notifications non-stop. Initially, 
unsigned long lastPlayTime = 0;

/************************************************************
  Audio objects
************************************************************/
AudioGeneratorMP3 *mp3 = NULL;
AudioFileSourceSD_MMC *file = NULL;
AudioFileSourceID3 *id3 = NULL;
AudioOutputI2SNoDAC *out = NULL;

/************************************************************
  BLE globals
************************************************************/
#define SERVICE_UUID        "4fafc201-1fb5-459e-8fcc-c5c9c331914b"
#define CHARACTERISTIC_UUID "beb5483e-36e1-4688-b7f5-ea07361b26a8"

BLECharacteristic* pCharacteristic;
bool deviceConnected = false;
int alertCount = 0;

class ServerCallbacks : public BLEServerCallbacks {
    void onConnect(BLEServer* pServer) {
        deviceConnected = true;
        Serial.println("Phone connected via BLE!");
    }
    void onDisconnect(BLEServer* pServer) {
        deviceConnected = false;
        Serial.println("Phone disconnected — re-advertising...");
        BLEDevice::startAdvertising();
    }
};

/************************************************************
  sendAlert()
  Call this anywhere to push a message to the phone. The phone connects through apps like LightBlue
  If no phone is connected this step is skipped.
  Source: Kolban, N. (2017). "BLE_notify.ino" [Source Code]. ESP32 BLE Arduino Library, Official GitHub Repository. github.com
  Modifications:
  We added a check to see if a phone is connected. If no phone is connected, it stops immediately and prints an error message to your computer, saving processing power.
  We added an Automatic Counter to keep track of the number of singals being sent.
  Added Text Customization.
  Added Computer Diagnostics

************************************************************/
void sendAlert(String reason) {
    if (!deviceConnected) {
        Serial.println("BLE: No phone connected — skipping alert");
        return;
    }
    alertCount++;
    String message = "Alert #" + String(alertCount) + ": " + reason;
    pCharacteristic->setValue(message.c_str());
    pCharacteristic->notify();
    Serial.println("BLE sent: " + message);
}

/************************************************************
  Audio metadata callback
************************************************************/
void MDCallback(void *cbData, const char *type, bool isUnicode,
                const char *string) {
    (void)cbData;
    Serial.printf("ID3 callback for: %s = '", type);
    if (isUnicode) string += 2;
    while (*string) {
        char a = *(string++);
        if (isUnicode) string++;
        Serial.printf("%c", a);
    }
    Serial.printf("'\n");
    Serial.flush();
}

/************************************************************
  handleAudio()
  Keeps the MP3 decoder running without blocking.
************************************************************/
void handleAudio() {
    if (mp3 != NULL && mp3->isRunning()) {
        if (!mp3->loop()) {
            mp3->stop();
            Serial.println("MP3 finished.");
        }
    }
}

/************************************************************
  getSonar(): this function handles the distance calculation. The ultrasound
  sensor sends a sound signal, detects the echo, and based on the speed of sound,
  calculated the distance traveled. Since the sound goes both ways, the actual distance 
  is obtained by dividing the result by two.
************************************************************/
float getSonar() {
    unsigned long pingTime;
    float distance;
    digitalWrite(trigPin, HIGH);
    delayMicroseconds(10);
    digitalWrite(trigPin, LOW);
    pingTime = pulseIn(echoPin, HIGH, timeOut);
    if (pingTime == 0) return -1;
    distance = (float)pingTime * soundVelocity / 2 / 10000;
    return distance;
}

/************************************************************
  getAverageDistance()

Takes multiple readings to avoid false triggers. This function helps mitigate the hypersensitivity of
the ultrasound sensor. The sensor is sensitive to any source that generates high-pitched sound waves,
air disruption, or electronic distruptions. So, we average out multiple measurements to reduce the effect of 
outlier distance measurements.
************************************************************/
float getAverageDistance(int readings) {
    float total = 0;
    int validCount = 0;
    for (int i = 0; i < readings; i++) {
        handleAudio();
        float distance = getSonar();
        handleAudio();
        if (distance > 0 && distance <= MAX_DISTANCE) {
            total += distance;
            validCount++;
        }
        unsigned long waitStart = millis();
        while (millis() - waitStart < 100) {
            handleAudio();
        }
    }
    if (validCount == 0) return -1;
    return total / validCount;
}

/************************************************************
  setupSDCard()
************************************************************/
void setupSDCard() {
    SD_MMC.setPins(SD_MMC_CLK, SD_MMC_CMD, SD_MMC_D0);
    if (!SD_MMC.begin("/sdcard", true, false, SDMMC_FREQ_DEFAULT, 5)) {
        Serial.println("Card Mount Failed");
        return;
    }
    uint8_t cardType = SD_MMC.cardType();
    if (cardType == CARD_NONE) {
        Serial.println("No SD_MMC card attached");
        return;
    }
    Serial.println("SD card initialized.");
    File testFile = SD_MMC.open(WELCOME_FILE);
    if (!testFile) {
        Serial.print("ERROR: Could not open ");
        Serial.println(WELCOME_FILE);
    } else {
        Serial.print("Found ");
        Serial.print(WELCOME_FILE);
        Serial.print(" | Size: ");
        Serial.print(testFile.size());
        Serial.println(" bytes");
        testFile.close();
    }
}

/************************************************************
  setupAudio()
************************************************************/
void setupAudio() {
    audioLogger = &Serial;
    out = new AudioOutputI2SNoDAC();
    out->SetPinout(16, 17, 14);
    out->SetGain(3.5);
    mp3 = new AudioGeneratorMP3();
    Serial.println("Audio initialized.");
}

/************************************************************
  setupBLE()
  This function sets up the ESP32 microcontroller to act as a Bluetooth Low Energy (BLE) Server.
  We chose BLE because it's the kind of bluetooth that phones are equipped with. This way, any mobile phone
  can connect with our ESP32 board.
  Based on the ESP32 BLE Notify Example by developer Neil Kolban
  Publisher: Programming Electronics AcademyArticle 
  Title: Making A BLE Server With Your ESP32 [Guide + Code]
  Video Source: ESP2 BLE Server in 69 Minutes!
  Website Group: Open Hardware Design Group LLC

  Modifications: 
  We added pServer->setCallbacks(new ServerCallbacks()); to let us know whenever a device connects or disconnects.
  We changed the rules to PROPERTY_NOTIFY and added new BLE2902() to allow the system to send live text-alerts.
************************************************************/
void setupBLE() {
    BLEDevice::init("ESP32_Receptionist");
    BLEServer* pServer = BLEDevice::createServer(); // Creates a BLE Server and waits for client devices (like a phone) to connect.
    pServer->setCallbacks(new ServerCallbacks());
    BLEService* pService = pServer->createService(SERVICE_UUID);
    pCharacteristic = pService->createCharacteristic(
        CHARACTERISTIC_UUID,
        BLECharacteristic::PROPERTY_READ |
        BLECharacteristic::PROPERTY_NOTIFY      // Lets it automatically push updates to the phone whenever the data changes
    );
    pCharacteristic->addDescriptor(new BLE2902());
    pService->start();
    BLEDevice::startAdvertising();    // Makes the device discoverable on bluetooth scanners
    Serial.println("BLE ready. Look for ESP32_Receptionist in LightBlue.");
}

/************************************************************
  playWelcome(): this function makes the speaker play the welcome message
************************************************************/
void playWelcome() {
    Serial.println("Playing welcome message...");
    if (mp3 != NULL && mp3->isRunning()) mp3->stop();
    if (id3 != NULL) { delete id3; id3 = NULL; }
    if (file != NULL) { delete file; file = NULL; }
    file = new AudioFileSourceSD_MMC(WELCOME_FILE);
    if (file == NULL || !file->isOpen()) {
        Serial.print("ERROR: Could not open ");
        Serial.println(WELCOME_FILE);
        return;
    }
    id3 = new AudioFileSourceID3(file);
    id3->RegisterMetadataCB(MDCallback, (void*)"ID3TAG");
    if (!mp3->begin(id3, out)) {
        Serial.println("ERROR: MP3 decoder could not start.");
        return;
    }
    Serial.println("MP3 started.");
}

/************************************************************
  setup()
************************************************************/
void setup() {
    Serial.begin(115200);
    delay(1000);
    Serial.println("Starting CS Commons Door Sensor...");

    pinMode(trigPin, OUTPUT);
    pinMode(echoPin, INPUT);
    digitalWrite(trigPin, LOW);

    setupSDCard();
    setupAudio();
    setupBLE();

    Serial.println("Calibrating doorway distance...");
    Serial.println("Make sure the doorway is empty.");
    delay(3000);

    baseDistance = getAverageDistance(10);      // calculates the base distance (estimate of the distance from the door)

    if (baseDistance < 0) {
        Serial.println("ERROR: Could not get starting doorway distance.");
    } else {
        Serial.print("Base doorway distance: ");
        Serial.print(baseDistance);
        Serial.println(" cm");
    }

    Serial.println("System ready. Connect your phone via LightBlue.");
}

/************************************************************
  loop()
Every ~300ms, the system:
    Measure distance
    Compare to baseline
    If movement detected:
        Play sound
        Send BLE alert
************************************************************/
void loop() {
    handleAudio();

    static unsigned long lastCheck = 0;

    if (millis() - lastCheck >= 300) {
        lastCheck = millis();

        float currentDistance = getAverageDistance(3);

        if (currentDistance < 0) {
            Serial.println("Bad ultrasonic reading.");
            return;
        }

        float difference = abs(currentDistance - baseDistance);    // To detect if there's been any changes in the distance
                                                                   // from the the sensor to the door. This will be compared against a treshold.

        Serial.print("Current distance: ");
        Serial.print(currentDistance);
        Serial.print(" cm | Difference: ");
        Serial.println(difference);

        if (difference >= movementThreshold) {        // Someone entered the door
            unsigned long currentTime = millis();

            if (currentTime - lastPlayTime >= cooldownTime) {
                Serial.println("Movement detected.");

                playWelcome();
                sendAlert("Someone entered the room!");

                lastPlayTime = currentTime;
            }
        }
    }
}