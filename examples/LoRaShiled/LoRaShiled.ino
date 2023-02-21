/**
 * @file      LoRaShiled.ino
 * @author    Lewis He (lewishe@outlook.com)
 * @license   MIT
 * @copyright Copyright (c) 2023  Shenzhen Xin Yuan Electronic Technology Co., Ltd
 * @date      2023-02-21
 *
 */

#include <Arduino.h>
#include <SPI.h>
#include <SD.h>
#include <Wire.h>
#include "LoRa.h"

/*
             LilyGo LoRa Shiled
        VDD
        GND

          .                  MOSI    --->    23
          .                  MISO    --->    15
  RST  ---> 12               SCK     --->    16
          VBAT               CS      --->    5
* */
#define SD_MISO             2
#define SD_MOSI             15
#define SD_SCLK             14
#define SD_Device_CS        13
#define LoRa_Device_CS      4       //Chip selection signal of another SPI
#define LoRa_Device_RST     12
#define LoRa_Device_DIO     39

// IO35,39,34,36 can only be used for input and cannot be set as output
#define I2C_SDA             33
#define I2C_SCL             32

int counter = 0;

void setup()
{
    Serial.begin(115200);

    pinMode(SD_MISO, INPUT_PULLUP);

    pinMode(LoRa_Device_CS, OUTPUT);

    // Disable another SPI device when accessing the SD card to ensure that the SD card is in normal orientation
    digitalWrite(SD_Device_CS, LOW);
    digitalWrite(LoRa_Device_CS, HIGH);

    SPI.begin(SD_SCLK, SD_MISO, SD_MOSI);

    while (1) {
        if (SD.begin(SD_Device_CS)) {
            Serial.println("SDCard MOUNT SUCCESS");
            break;
        }
        Serial.println("SDCard MOUNT FAIL");
        delay(500);
    }

    // When accessing other spi devices, disable the SD card to ensure normal access to other devices
    digitalWrite(SD_Device_CS, HIGH);

    // Select another SPI device
    LoRa.setPins(LoRa_Device_CS, LoRa_Device_RST, LoRa_Device_DIO);
    //Examples accessing other spi devices
    if (!LoRa.begin(915E6)) {
        while (1) {
            Serial.println("Starting LoRa failed!");
            delay(1000);
        }
    }
    // Maximize the use of available IO
    Wire.begin(I2C_SDA, I2C_SCL);
    //....I2C Devices do someing ..

}

void loop()
{
    Serial.print("Sending packet: ");
    Serial.println(counter);

    // send packet
    LoRa.beginPacket();
    LoRa.print("hello ");
    LoRa.print(counter);
    LoRa.endPacket();

    counter++;

    delay(1000);
}