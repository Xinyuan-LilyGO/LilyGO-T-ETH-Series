#include <Arduino.h>
#include <Wire.h>



#define I2C_SDA 14
#define I2C_SCL 15

void setup()
{
    // Using Wire requires an explicit call
    Wire.begin(I2C_SDA, I2C_SCL);
    // do someing ..
}

void loop()
{

}