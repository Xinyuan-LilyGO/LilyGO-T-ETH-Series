/**
 * @file      I2C_BME280Sensor.ino
 * @author    Lewis He (lewishe@outlook.com)
 * @license   MIT
 * @copyright Copyright (c) 2024  ShenZhen XinYuan Electronic Technology Co., Ltd
 * @date      2024-06-24
 * @note      The BME280 sensor is currently only available in T-ETH-ELite-LoRa-Shield. 
 *            Other models do not have it and require external wiring to the sensor module.
 */

#include <Wire.h>
#include <SPI.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>
#include <U8g2lib.h>

// Select the board model to be used and adjust the Pin according to the actual situation

// #define LILYGO_T_INTERNET_POE        //There is no BME280 sensor by default, and an external wire is required to connect to the module
// #define LILYGO_T_ETH_POE_PRO         //There is no BME280 sensor by default, and an external wire is required to connect to the module
// #define LILYGO_T_INTER_COM           //Can't run
// #define LILYGO_T_ETH_LITE_ESP32      //There is no BME280 sensor by default, and an external wire is required to connect to the module
// #define LILYGO_T_ETH_LITE_ESP32S3    //There is no BME280 sensor by default, and an external wire is required to connect to the module
// #define LILYGO_T_ETH_ELITE_ESP32S3   //Default [T-ETH-ELite-LoRa-Shield onboard BME280 sensor] ,The main board alone has no sensor 


#if   defined(LILYGO_T_INTERNET_POE)
// IO34 35,39,34,36 can only be used for input and cannot be set as output
#define I2C_SDA     14
#define I2C_SCL     15
#elif defined(LILYGO_T_ETH_POE_PRO)
// IO34 35,39,34,36 can only be used for input and cannot be set as output
#define I2C_SDA     13
#define I2C_SCL     14
#elif defined(LILYGO_T_INTER_COM)
//No free pin
#error "No  free pin"
#elif defined(LILYGO_T_ETH_LITE_ESP32)
// IO35,36,37,38,39 can only be used for input and cannot be set as output
#define I2C_SDA     13
#define I2C_SCL     14
#elif defined(LILYGO_T_ETH_LITE_ESP32S3)
// ESP32S3 can freely map unused Pins
#define I2C_SDA     1
#define I2C_SCL     2
#elif defined(LILYGO_T_ETH_ELITE_ESP32S3)
// ESP32S3 can freely map unused Pins , If you use an expansion board, you can only use SDA 17,SCL 18
#define I2C_SDA     17
#define I2C_SCL     18
#endif

// OLED model 
#define DISPLAY_MODEL               U8G2_SSD1306_128X64_NONAME_F_HW_I2C
// OLED device address
#define DISPLAY_ADDR                0x3C

DISPLAY_MODEL *u8g2 = NULL;

#define SEALEVELPRESSURE_HPA (1013.25)

Adafruit_BME280 bme; // I2C

unsigned long delayTime;


uint32_t deviceScan(TwoWire *_port, Stream *stream)
{
    stream->println("Devices Scan start.");
    uint8_t err, addr;
    int nDevices = 0;
    for (addr = 1; addr < 127; addr++) {
        _port->beginTransmission(addr);
        err = _port->endTransmission();
        if (err == 0) {
            stream->print("I2C device found at address 0x");
            if (addr < 16)
                stream->print("0");
            stream->print(addr, HEX);
            stream->println(" !");
            nDevices++;
        } else if (err == 4) {
            stream->print("Unknow error at address 0x");
            if (addr < 16)
                stream->print("0");
            stream->println(addr, HEX);
        }
    }
    if (nDevices == 0)
        stream->println("No I2C devices found\n");
    else
        stream->println("Done\n");
    return nDevices;
}


void setup()
{
    Serial.begin(115200);
    while (!Serial);   // time to get serial running
    Serial.println(F("BME280 test"));

    Wire.begin(I2C_SDA_PIN, I2C_SCL_PIN);

    deviceScan(&Wire, &Serial);


    // default settings
    bool status = bme.begin();
    // You can also pass in a Wire library object like &Wire2
    // status = bme.begin(0x76, &Wire2)
    if (!status) {
        Serial.println("Could not find a valid BME280 sensor, check wiring, address, sensor ID!");
        Serial.print("SensorID was: 0x"); Serial.println(bme.sensorID(), 16);
        Serial.print("        ID of 0xFF probably means a bad address, a BMP 180 or BMP 085\n");
        Serial.print("   ID of 0x56-0x58 represents a BMP 280,\n");
        Serial.print("        ID of 0x60 represents a BME 280.\n");
        Serial.print("        ID of 0x61 represents a BME 680.\n");
        if (u8g2) {
            u8g2->setFont(u8g2_font_ncenB08_tr);
            u8g2->clearBuffer();
            u8g2->setCursor(0, 16);
            u8g2->print("BME280 Could not find");
            u8g2->sendBuffer();
        }
        while (1) delay(10);
    }

    Wire.beginTransmission(DISPLAY_ADDR);
    if (Wire.endTransmission() == 0) {
        Serial.printf("Find Display model at 0x%X address\n", DISPLAY_ADDR);
        u8g2 = new DISPLAY_MODEL(U8G2_R0, U8X8_PIN_NONE);
        u8g2->begin();
    }

    Serial.println("-- Default Test --");
    delayTime = 1000;

    Serial.println();
}


void loop()
{
    printValues();
    delay(delayTime);
}


void printValues()
{
    Serial.print("Temperature = ");
    Serial.print(bme.readTemperature());
    Serial.println(" °C");

    Serial.print("Pressure = ");

    Serial.print(bme.readPressure() / 100.0F);
    Serial.println(" hPa");

    Serial.print("Approx. Altitude = ");
    Serial.print(bme.readAltitude(SEALEVELPRESSURE_HPA));
    Serial.println(" m");

    Serial.print("Humidity = ");
    Serial.print(bme.readHumidity());
    Serial.println(" %");

    Serial.println();

    if (u8g2) {
        u8g2->setFont(u8g2_font_ncenB08_tr);
        u8g2->clearBuffer();

        u8g2->setCursor(0, 16);

        u8g2->print("Temperature:");
        u8g2->print(bme.readTemperature());
        u8g2->print(" *C");

        u8g2->setCursor(0, 32);
        u8g2->print("Pressure:");
        u8g2->println(bme.readPressure() / 100.0F);
        u8g2->print(" hPa");

        u8g2->setCursor(0, 48);
        u8g2->print("Altitude:");
        u8g2->println(bme.readAltitude(SEALEVELPRESSURE_HPA));
        u8g2->print(" m");

        u8g2->setCursor(0, 64);
        u8g2->print("Humidity:");
        u8g2->println(bme.readHumidity());
        u8g2->print(" %");

        u8g2->sendBuffer();
    }
}
