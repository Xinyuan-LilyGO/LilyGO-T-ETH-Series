/**
 * @file      I2C_HP303BSensor.ino
 * @author    Lewis He (lewishe@outlook.com)
 * @license   MIT
 * @copyright Copyright (c) 2024  Shenzhen Xin Yuan Electronic Technology Co., Ltd
 * @date      2024-06-05
 * @note      Connect wemos hp303b temperature and pressure sensor,
 *            the sensor is not included in ETH Board, the sketch only demonstrates how to use Wire
 */
#include "src/LOLIN_HP303B.h"       //https://github.com/wemos/LOLIN_HP303B_Library

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

LOLIN_HP303B HP303BPressureSensor;

// Default HP303B Address 0x77 , may be is 0x76
const uint8_t slaveAddress = 0x77;

void scanDevices(TwoWire *w)
{
    uint8_t err, addr;
    int nDevices = 0;
    uint32_t start = 0;
    for (addr = 1; addr < 127; addr++) {
        start = millis();
        w->beginTransmission(addr); delay(2);
        err = w->endTransmission();
        if (err == 0) {
            nDevices++;
            Serial.print("I2C device found at address 0x");
            if (addr < 16) {
                Serial.print("0");
            }
            Serial.print(addr, HEX);
            Serial.println(" !");
        } else if (err == 4) {
            Serial.print("Unknow error at address 0x");
            if (addr < 16) {
                Serial.print("0");
            }
            Serial.println(addr, HEX);
        }
    }
    if (nDevices == 0)
        Serial.println("No I2C devices found\n");
}


void setup(void)
{
    Serial.begin(115200);

    Wire.begin(I2C_SDA, I2C_SCL);

    Serial.println("Scan devices ....");

    scanDevices(&Wire);

    HP303BPressureSensor.begin(slaveAddress);
}

void loop()
{
    int32_t temperature;
    int32_t pressure;
    int16_t oversampling = 7;
    int16_t ret, ret1;

    Serial.println();

    //lets the HP303B perform a Single temperature measurement with the last (or standard) configuration
    //The result will be written to the paramerter temperature
    //ret = HP303BPressureSensor.measureTempOnce(temperature);
    //the commented line below does exactly the same as the one above, but you can also config the precision
    //oversampling can be a value from 0 to 7
    //the HP303B will perform 2^oversampling internal temperature measurements and combine them to one result with higher precision
    //measurements with higher precision take more time, consult datasheet for more information
    ret = HP303BPressureSensor.measureTempOnce(temperature, oversampling);

    if (ret != 0) {
        //Something went wrong.
        //Look at the library code for more information about return codes
        Serial.print("FAIL! ret = ");
        Serial.println(ret);
    } else {
        Serial.print("Temperature: ");
        Serial.print(temperature);
        Serial.println(" degrees of Celsius");
    }

    //Pressure measurement behaves like temperature measurement
    //ret = HP303BPressureSensor.measurePressureOnce(pressure);
    ret1 = HP303BPressureSensor.measurePressureOnce(pressure, oversampling);
    if (ret1 != 0) {
        //Something went wrong.
        //Look at the library code for more information about return codes
        Serial.print("FAIL! ret = ");
        Serial.println(ret1);
    } else {
        Serial.print("Pressure: ");
        Serial.print(pressure);
        Serial.println(" Pascal");
    }

    delay(1000);
}

