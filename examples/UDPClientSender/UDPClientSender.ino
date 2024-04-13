/**
 * @file      UDPClient.ino
 * @author    Lewis He (lewishe@outlook.com)
 * @license   MIT
 * @copyright Copyright (c) 2023  Shenzhen Xin Yuan Electronic Technology Co., Ltd
 * @date      2023-02-17
 *
 */
#include <Arduino.h>
#if ESP_ARDUINO_VERSION < ESP_ARDUINO_VERSION_VAL(3,0,0)
#include <ETHClass2.h>       //Is to use the modified ETHClass
#define ETH  ETH2
#else
#include <ETH.h>
#endif
#include <SPI.h>
#include <SD.h>
#include "utilities.h"          //Board PinMap
#include <WiFi.h>

//The udp library class
WiFiUDP udp;

//IP address to send UDP data to:
// either use the ip address of the server or
// a network broadcast address
const char *udpAddress = "192.168.36.76";
const int udpPort = 3333;


static bool eth_connected = false;

void WiFiEvent(arduino_event_id_t event)
{
    switch (event) {
    case ARDUINO_EVENT_ETH_START:
        Serial.println("ETH Started");
        //set eth hostname here
        ETH.setHostname("esp32-ethernet");
        break;
    case ARDUINO_EVENT_ETH_CONNECTED:
        Serial.println("ETH Connected");
        break;
    case ARDUINO_EVENT_ETH_GOT_IP:
        Serial.print("ETH MAC: ");
        Serial.print(ETH.macAddress());
        Serial.print(", IPv4: ");
        Serial.print(ETH.localIP());
        if (ETH.fullDuplex()) {
            Serial.print(", FULL_DUPLEX");
        }

        Serial.print(", ");
        Serial.print(ETH.linkSpeed());
        Serial.println("Mbps");
        eth_connected = true;

        //initializes the UDP state
        //This initializes the transfer buffer
        udp.begin(WiFi.localIP(), udpPort);


        break;
    case ARDUINO_EVENT_ETH_DISCONNECTED:
        Serial.println("ETH Disconnected");
        eth_connected = false;
        break;
    case ARDUINO_EVENT_ETH_STOP:
        Serial.println("ETH Stopped");
        eth_connected = false;
        break;
    default:
        break;
    }
}


void setup()
{
    Serial.begin(115200);

    WiFi.onEvent(WiFiEvent);

#ifdef ETH_POWER_PIN
    pinMode(ETH_POWER_PIN, OUTPUT);
    digitalWrite(ETH_POWER_PIN, HIGH);
#endif


#if CONFIG_IDF_TARGET_ESP32
    if (!ETH.begin(ETH_TYPE, ETH_ADDR, ETH_MDC_PIN,
                   ETH_MDIO_PIN, ETH_RESET_PIN, ETH_CLK_MODE)) {
        Serial.println("ETH start Failed!");
    }
#else
    if (!ETH.begin(ETH_PHY_W5500, 1, ETH_CS_PIN, ETH_INT_PIN, ETH_RST_PIN,
                   SPI3_HOST,
                   ETH_SCLK_PIN, ETH_MISO_PIN, ETH_MOSI_PIN)) {
        Serial.println("ETH start Failed!");
    }
#endif

}

void loop()
{
    if (eth_connected) {
        //Send a packet
        udp.beginPacket(udpAddress, udpPort);
        udp.printf("Seconds since boot: %lu", millis() / 1000);
        udp.endPacket();

    }
    delay(1000);
}