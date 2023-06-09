/**
 * @file      UDPClient.ino
 * @author    Lewis He (lewishe@outlook.com)
 * @license   MIT
 * @copyright Copyright (c) 2023  Shenzhen Xin Yuan Electronic Technology Co., Ltd
 * @date      2023-02-17
 *
 */
#include <Arduino.h>
#include <ETH.h>
#include <SPI.h>
#include <SD.h>

#undef ETH_CLK_MODE

// #define LILYGO_INTERNET_COM          //Uncomment will use LilyGo-Internet-COM's pinmap

#ifdef LILYGO_INTERNET_COM
#define ETH_CLK_MODE        ETH_CLOCK_GPIO0_OUT
#define ETH_POWER_PIN       4
#else
#define ETH_CLK_MODE        ETH_CLOCK_GPIO17_OUT
#define ETH_POWER_PIN       5
#endif

#define ETH_TYPE            ETH_PHY_LAN8720
#define ETH_ADDR            0
#define ETH_MDC_PIN         23
#define ETH_MDIO_PIN        18
#define SD_MISO             2
#define SD_MOSI             15
#define SD_SCLK             14
#define SD_CS               13

//The udp library class
WiFiUDP udp;

//IP address to send UDP data to:
// either use the ip address of the server or
// a network broadcast address
const char *udpAddress = "192.168.36.76";
const int udpPort = 3333;


static bool eth_connected = false;

void WiFiEvent(WiFiEvent_t event)
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




    ETH.begin(ETH_ADDR, ETH_POWER_PIN, ETH_MDC_PIN,
              ETH_MDIO_PIN, ETH_TYPE, ETH_CLK_MODE);

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