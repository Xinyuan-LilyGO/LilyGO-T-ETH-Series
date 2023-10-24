/**
 * @file      WireExample.ino
 * @author    Lewis He (lewishe@outlook.com)
 * @license   MIT
 * @copyright Copyright (c) 2023  Shenzhen Xinyuan Electronic Technology Co., Ltd
 * @date      2023-07-27
 * @note      The sketch just demonstrates how to use a Wire
 */

#include <Arduino.h>
#include <Wire.h>




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
// IO35,39,34,36 can only be used for input and cannot be set as output
#define I2C_SDA     13
#define I2C_SCL     14
#elif defined(LILYGO_T_ETH_LITE_ESP32S3)
// ESP32S3 can freely map unused Pins
#define I2C_SDA     1
#define I2C_SCL     2
#endif


void setup()
{
    // Using Wire requires an explicit call
    Wire.begin(I2C_SDA, I2C_SCL);
    // do someing ..
}

void loop()
{

}