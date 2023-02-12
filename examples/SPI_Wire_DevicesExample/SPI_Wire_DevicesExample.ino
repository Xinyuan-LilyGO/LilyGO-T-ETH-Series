/**
 * @file      SPI_Wire_DevicesExample.ino
 * @author    Lewis He (lewishe@outlook.com)
 * @license   MIT
 * @copyright Copyright (c) 2023  Shenzhen Xinyuan Electronic Technology Co., Ltd
 * @date      2023-02-12
 *
 */

/*
*       Hardware connection
*    ESP32          LoRa
*   GPIO14 ---->   CLK
*   GPIO2  ---->   MISO
*   GPIO15 ---->   MOSI
*   GPIO4  ---->    CS
*   GPIO16 ---->   RST
*   GPIO36 ---->   DIO
*
*    ESP32        OLED
*   GPIO33 ---->   SDA
*   GPIO32 ---->   SCL
* */
// IO35,39,34,36 can only be used for input and cannot be set as output


#include <Arduino.h>
#include <SPI.h>
#include <SD.h>
#include <Wire.h>
#include <LoRa.h>
#include <U8g2lib.h>

#define SD_MISO                 2
#define SD_MOSI                 15
#define SD_SCLK                 14
#define SD_Device_CS            13
#define LoRa_Device_CS          4           // LoRa radio chip select
#define LoRa_Device_RST         16          // LoRa radio reset
#define LoRa_Device_DIO         36          // LoRa  interrupt


#define I2C_SDA                 33
#define I2C_SCL                 32

U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, U8X8_PIN_NONE);

String outgoing;              // outgoing message

byte msgCount = 0;            // count of outgoing messages
byte localAddress = 0xBB;     // address of this device
byte destination = 0xFF;      // destination to send to
long lastSendTime = 0;        // last send time
int interval = 2000;          // interval between sends


void sendMessage(String outgoing)
{
    LoRa.beginPacket();                   // start packet
    LoRa.write(destination);              // add destination address
    LoRa.write(localAddress);             // add sender address
    LoRa.write(msgCount);                 // add message ID
    LoRa.write(outgoing.length());        // add payload length
    LoRa.print(outgoing);                 // add payload
    LoRa.endPacket();                     // finish packet and send it
    msgCount++;                           // increment message ID
}

void onReceive(int packetSize)
{
    if (packetSize == 0) return;          // if there's no packet, return

    // read packet header bytes:
    int recipient = LoRa.read();          // recipient address
    byte sender = LoRa.read();            // sender address
    byte incomingMsgId = LoRa.read();     // incoming msg ID
    byte incomingLength = LoRa.read();    // incoming msg length

    String incoming = "";

    while (LoRa.available()) {
        incoming += (char)LoRa.read();
    }

    if (incomingLength != incoming.length()) {   // check length for error
        Serial.println("error: message length does not match length");
        return;                             // skip rest of function
    }

    // if the recipient isn't this device or broadcast,
    if (recipient != localAddress && recipient != 0xFF) {
        Serial.println("This message is not for me.");
        return;                             // skip rest of function
    }

    // if message is for this device, or broadcast, print details:
    Serial.println("Received from: 0x" + String(sender, HEX));
    Serial.println("Sent to: 0x" + String(recipient, HEX));
    Serial.println("Message ID: " + String(incomingMsgId));
    Serial.println("Message length: " + String(incomingLength));
    Serial.println("Message: " + incoming);
    Serial.println("RSSI: " + String(LoRa.packetRssi()));
    Serial.println("Snr: " + String(LoRa.packetSnr()));
    Serial.println();
}



void setup()
{
    Serial.begin(115200);

    pinMode(SD_MISO, INPUT_PULLUP);

    pinMode(LoRa_Device_CS, OUTPUT);

    // Disable Lora SPI device when accessing the SD card to ensure that the SD card is in normal orientation
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

    // Select LoRa SPI device
    digitalWrite(LoRa_Device_CS, LOW);

    // override the default CS, reset, and IRQ pins (optional)
    LoRa.setPins(LoRa_Device_CS, LoRa_Device_RST, LoRa_Device_DIO);// set CS, reset, IRQ pin

    if (!LoRa.begin(915E6)) {             // initialize ratio at 915 MHz
        Serial.println("LoRa init failed. Check your connections.");
        while (true);                       // if failed, do nothing
    }


    // Maximize the use of available IO
    Wire.begin(I2C_SDA, I2C_SCL);

    u8g2.begin();

    u8g2.clearBuffer();                   // clear the internal memory
    u8g2.setFont(u8g2_font_ncenB08_tr);   // choose a suitable font
    u8g2.drawStr(0, 10, "Hello World!");  // write something to the internal memory
    u8g2.sendBuffer();                    // transfer internal memory to the display

}

void loop()
{
    if (millis() - lastSendTime > interval) {
        String message = "HeLoRa World!";   // send a message
        sendMessage(message);
        Serial.println("Sending " + message);
        lastSendTime = millis();            // timestamp the message
        interval = random(2000) + 1000;    // 2-3 seconds
    }

    // parse for a packet, and call onReceive with the result:
    onReceive(LoRa.parsePacket());
}