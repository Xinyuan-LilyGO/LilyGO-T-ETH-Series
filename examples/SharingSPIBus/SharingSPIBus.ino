/**
 * @file      SharingSPIBus.ino
 * @author    Lewis He (lewishe@outlook.com)
 * @license   MIT
 * @copyright Copyright (c) 2023  Shenzhen Xinyuan Electronic Technology Co., Ltd
 * @date      2023-02-12
 * @note      The sketch only demonstrates how to multiplex the SPI bus with T-ETH-POE. Other boards can refer to
 */
#include <Arduino.h>
#include <SPI.h>
#include <SD.h>
#include <Wire.h>

#define SD_MISO_PIN             2
#define SD_MOSI_PIN             15
#define SD_SCLK_PIN             14
#define SD_Device_CS            13
#define Other_Device_CS         4       //Chip selection signal of another SPI

// IO35,39,34,36 can only be used for input and cannot be set as output

#define I2C_SDA                 33
#define I2C_SCL                 32

// The sketch only demonstrates how to multiplex the SPI bus with T-ETH-POE. Other boards can refer to
void setup()
{
    Serial.begin(115200);

    pinMode(SD_MISO_PIN, INPUT_PULLUP);

    pinMode(Other_Device_CS, OUTPUT);
    // Disable another SPI device when accessing the SD card to ensure that the SD card is in normal orientation
    digitalWrite(Other_Device_CS, HIGH);

    SPI.begin(SD_SCLK_PIN, SD_MISO_PIN, SD_MOSI_PIN);
    while (1) {
        if (SD.begin(SD_Device_CS)) {
            Serial.println("SDCard MOUNT SUCCESS");
            break;
        }
        Serial.println("SDCard MOUNT FAIL");
        delay(500);
    }

    // When accessing the SD card, the CS of other SPI devices must be set to HIGH
    digitalWrite(SD_Device_CS, HIGH);

    // Select another SPI device
    digitalWrite(Other_Device_CS, LOW);

    //Examples accessing other spi devices
    uint8_t data = 0x00;
    SPI.beginTransaction(SPISettings());
    SPI.write(data);
    SPI.endTransaction();

    // Release SPI device access
    digitalWrite(Other_Device_CS, HIGH);


    // Maximize the use of available IO
    Wire.begin(I2C_SDA, I2C_SCL);

    //....I2C Devices do someing ..
}

void loop()
{

}