/**
 * @file      SharingSPIBus.ino
 * @author    Lewis He (lewishe@outlook.com)
 * @license   MIT
 * @copyright Copyright (c) 2023  Shenzhen Xinyuan Electronic Technology Co., Ltd
 * @date      2023-02-12
 *
 */
#include <Arduino.h>
#include <SPI.h>
#include <SD.h>
#include <Wire.h>

#define SD_MISO             2
#define SD_MOSI             15
#define SD_SCLK             14
#define SD_Device_CS        13
#define Other_Device_CS      4       //Chip selection signal of another SPI

// IO35,39,34,36 can only be used for input and cannot be set as output

#define I2C_SDA     33
#define I2C_SCL     32

void setup()
{
    Serial.begin(115200);

    pinMode(SD_MISO, INPUT_PULLUP);

    pinMode(Other_Device_CS, OUTPUT);
    // Disable another SPI device when accessing the SD card to ensure that the SD card is in normal orientation
    digitalWrite(SD_Device_CS, HIGH);

    SPI.begin(SD_SCLK, SD_MISO, SD_MOSI, SD_Device_CS);
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
    digitalWrite(Other_Device_CS, LOW);

    //Examples accessing other spi devices
    uint8_t data = 0x00;
    SPI.beginTransaction(SPISettings());
    SPI.write(data);
    SPI.endTransaction();


    // Maximize the use of available IO
    Wire.begin(I2C_SDA, I2C_SCL);

    //....I2C Devices do someing ..
}

void loop()
{

}