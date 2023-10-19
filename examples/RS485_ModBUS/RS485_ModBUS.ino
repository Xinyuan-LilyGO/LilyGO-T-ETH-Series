/**
 * @file      RS485_ModBUS.ino
 * @author    Lewis He (lewishe@outlook.com)
 * @license   MIT
 * @copyright Copyright (c) 2023  Shenzhen Xin Yuan Electronic Technology Co., Ltd
 * @date      2023-10-19
 * @note      This sketch is only suitable for ETH-POE-PRO, other boards do not have RS485 function
 */
#include <Arduino.h>
#include <ModbusMaster.h>

#define SerialMon   Serial
#define Serial485   Serial2

#define BOARD_485_TX                33
#define BOARD_485_RX                32


ModbusMaster node;

uint32_t runningMillis = 0;
uint32_t counter = 0;

void setup()
{
    // Set console baud rate
    SerialMon.begin(115200);

    Serial485.begin(9600, SERIAL_8N1, BOARD_485_RX, BOARD_485_TX);

    // communicate with Modbus slave ID 2 over Serial485
    node.begin(2, Serial485);

}

void loop()
{
    static uint32_t i;
    uint8_t j, result;
    uint16_t data[6];

    i++;

    // set word 0 of TX buffer to least-significant word of counter (bits 15..0)
    node.setTransmitBuffer(0, lowWord(i));

    // set word 1 of TX buffer to most-significant word of counter (bits 31..16)
    node.setTransmitBuffer(1, highWord(i));

    // slave: write TX buffer to (2) 16-bit registers starting at register 0
    result = node.writeMultipleRegisters(0, 2);

    // slave: read (6) 16-bit registers starting at register 2 to RX buffer
    result = node.readHoldingRegisters(2, 6);

    // do something with data if read is successful
    if (result == node.ku8MBSuccess) {
        for (j = 0; j < 6; j++) {
            data[j] = node.getResponseBuffer(j);
        }
    }
}