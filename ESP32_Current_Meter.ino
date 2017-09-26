/*
   Connect the SD card to the following pins:

   SD Card | ESP32
      D2       -
      D3       SS
      CMD      MOSI
      VSS      GND
      VDD      3.3V
      CLK      SCK
      VSS      GND
      D0       MISO
      D1       -
*/
#include "FS.h"
#include "SD.h"
#include "SPI.h"
#include <Wire.h>
#include <Adafruit_INA219.h>

Adafruit_INA219 ina219_1;
Adafruit_INA219 ina219_2;

String fname1 = "";
char fname2[128] = "";

void setup() {
  Serial.begin(115200);
  delay(500);

  if (!SD.begin()) {
    Serial.println("Card Mount Failed");
    return;
  }

  uint8_t cardType = SD.cardType();
  if (cardType == CARD_NONE) {
    Serial.println("No SD card attached");
    return;
  }
  Serial.print("SD Card Type: ");
  if (cardType == CARD_MMC) {
    Serial.println("MMC");
  } else if (cardType == CARD_SD) {
    Serial.println("SDSC");
  } else if (cardType == CARD_SDHC) {
    Serial.println("SDHC");
  } else {
    Serial.println("UNKNOWN");
  }

  uint64_t cardSize = SD.cardSize() / (1024 * 1024);
  Serial.printf("SD Card Size: %lluMB\n", cardSize);

  Serial.println();
  listDir(SD, "/", 0);

  Serial.println("Date: " + String(__DATE__));
  Serial.println("Time: " + String(__TIME__));

  fname1 = "log_" + String(__DATE__) + "_" + String(__TIME__) + ".txt";
  fname1.replace(" ", "_");
  fname1.replace(":", "_");

  ina219_1.begin(0x44);
  ina219_1.setCalibration_16V_400mA();
  ina219_2.begin(0x40);
  ina219_2.setCalibration_16V_400mA();

  Serial.println();
  Serial.println("Creating new file: " + fname1);

  sprintf(fname2, "/%s", fname1.c_str());
  writeFile(SD, fname2, "TIME\tuA\tuA\n");
}

void loop() {
  //float sv1 = ina219_1.getShuntVoltage_mV();
  //float bv1 = ina219_1.getBusVoltage_V() * 1000.0;
  //float lv1 = bv1 + sv1;
  float cc1 = ina219_1.getCurrent_mA() * 1000.0;
  //float sv2 = ina219_2.getShuntVoltage_mV();
  //float bv2 = ina219_2.getBusVoltage_V() * 1000.0;
  //float lv2 = bv2 + sv2;
  float cc2 = ina219_2.getCurrent_mA() * 1000.0;
  char fdata[128] = "";
  sprintf(fdata, "%D\t%5.1F\t%5.1F\n", millis(), cc1, cc2);
  appendFile(SD, fname2, fdata);
  Serial.print(fdata);
  delay(100);
}

void writeFile(fs::FS &fs, const char * path, const char * message) {
  Serial.printf("Writing file: %s\n", path);

  File file = fs.open(path, FILE_WRITE);
  if (!file) {
    Serial.println("Failed to open file for writing");
    return;
  }
  if (file.print(message)) {
    //Serial.println("File written");
  } else {
    Serial.println("Write failed");
  }
  file.close();
}

void appendFile(fs::FS &fs, const char * path, const char * message) {
  //Serial.printf("Appending to file: %s\n", path);

  File file = fs.open(path, FILE_APPEND);
  if (!file) {
    Serial.println("Failed to open file for appending");
    return;
  }
  if (file.print(message)) {
    //Serial.println("Message appended");
  } else {
    Serial.println("Append failed");
  }
  file.close();
}

void listDir(fs::FS &fs, const char * dirname, uint8_t levels) {
  Serial.printf("Listing directory: %s\n", dirname);

  File root = fs.open(dirname);
  if (!root) {
    Serial.println("Failed to open directory");
    return;
  }
  if (!root.isDirectory()) {
    Serial.println("Not a directory");
    return;
  }

  File file = root.openNextFile();
  while (file) {
    if (file.isDirectory()) {
      Serial.print("  DIR : ");
      Serial.println(file.name());
      if (levels) {
        listDir(fs, file.name(), levels - 1);
      }
    } else {
      Serial.print("  FILE: ");
      Serial.print(file.name());
      Serial.print("  SIZE: ");
      Serial.println(file.size());
    }
    file = root.openNextFile();
  }
}

