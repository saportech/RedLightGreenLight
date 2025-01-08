#include <SD.h>
#include <Audio.h>
#include <FS.h>
#include "SPI.h"
#include "IMU.h"

#define SD_CS          5
#define SPI_MOSI      23    // SD Card
#define SPI_MISO      19
#define SPI_SCK       18

#define I2S_DOUT 33
#define I2S_BCLK 26
#define I2S_LRC 25
#define SD_CS 5

Audio audio;
IMU imu;

void setupAudio();
void runAudio();

void setup() {
  Serial.begin(115200);
  while(!Serial) {}

  Serial.println("Starting setup...");
  delay(3000);

  imu.begin();

  //setupAudio();
}

void loop() {

    // audio.loop();
    // runAudio();

}

void runAudio() {
  if (!audio.isRunning()) {
    if (!audio.connecttoFS(SD, "/001.mp3")) {
      Serial.println("Failed to open audio file");
    } else {
      Serial.println("Playing audio file: 001.mp3");
    }
  }
}

void setupAudio() {

  pinMode(SD_CS, OUTPUT);
  digitalWrite(SD_CS, HIGH);
  SPI.begin(SPI_SCK, SPI_MISO, SPI_MOSI);

  while (!SD.begin(SD_CS)) {
    Serial.println("Trying to initialize SD card...");
    delay(1000);
  }
  Serial.println("SD card initialized.");

  audio.setPinout(I2S_BCLK, I2S_LRC, I2S_DOUT);
  audio.setVolume(21);

  if (!audio.connecttoFS(SD, "/001.mp3")) {
    Serial.println("Failed to open audio file");
    while (true);
  }
  Serial.println("Playing audio file: 001.mp3");
}
