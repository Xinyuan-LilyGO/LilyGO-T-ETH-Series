/**
 * @file      RS485_Master.ino
 * @author    Lewis He (lewishe@outlook.com)
 * @license   MIT
 * @copyright Copyright (c) 2023  Shenzhen Xin Yuan Electronic Technology Co., Ltd
 * @date      2023-07-27
 * @note      This sketch is only suitable for ETH-POE-PRO, other boards do not have RS485 function
 */
#include <Arduino.h>

#define SerialMon   Serial
#define Serial485   Serial2

#define BOARD_485_TX                33
#define BOARD_485_RX                32


uint32_t runningMillis = 0;
uint32_t counter = 0;

void setup()
{
    // Set console baud rate
    SerialMon.begin(115200);

    Serial485.begin(9600, SERIAL_8N1, BOARD_485_RX, BOARD_485_TX);

}

void loop()
{
    if (runningMillis < millis()) {
        runningMillis = millis() + 2000;
        Serial485.print("[");
        Serial485.print(millis());
        Serial485.print("]");
        Serial485.print(" Counter:");
        Serial485.println(counter++);

        Serial.print("Sender -> [");
        Serial.print(millis());
        Serial.print("]");
        Serial.print(" Millis:");
        Serial.println(counter++);

    }

    while (Serial485.available()) {
        SerialMon.write(Serial485.read());
    }
    while (SerialMon.available()) {
        Serial485.write(SerialMon.read());
    }

}