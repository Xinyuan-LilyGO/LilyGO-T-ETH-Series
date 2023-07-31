/**
 * @file      RS485_Slave.ino
 * @author    Lewis He (lewishe@outlook.com)
 * @license   MIT
 * @copyright Copyright (c) 2023  Shenzhen Xin Yuan Electronic Technology Co., Ltd
 * @date      2023-07-27
 * @note      This sketch is only suitable for ETH-POE-PRO, other boards do not have RS485 function
 */
#include <Arduino.h>

#define BOARD_485_TX                33
#define BOARD_485_RX                32

#define Serial485 Serial2

uint32_t runningMillis = 0;
uint32_t counter = 0;

void setup()
{
    Serial.begin(115200);
    Serial485.begin(9600, SERIAL_8N1, BOARD_485_RX, BOARD_485_TX);
}


void loop()
{
    while (Serial.available()) {
        Serial485.write(Serial.read());
    }
    while (Serial485.available()) {
        String  recv = Serial485.readStringUntil('\n');
        Serial.print("<- Recver:");
        Serial.println(recv);
        Serial485.print("<-- Rspone :"); Serial485.println(recv);
    }
}






