/**
 * @file      TCPServer.ino
 * @author    Lewis He (lewishe@outlook.com)
 * @license   MIT
 * @copyright Copyright (c) 2023  Shenzhen Xin Yuan Electronic Technology Co., Ltd
 * @date      2023-06-08
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


//TCP Server listen port
#define TCP_SERVER_PORT     80


WiFiServer server(TCP_SERVER_PORT);
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

    server.begin();
}

void loop()
{
    if (eth_connected) {
        WiFiClient client = server.available();         // listen for incoming clients
        if (client) {                                   // if you get a client,
            Serial.println("New Client.");              // print a message out the serial port
            String currentLine = "";                    // make a String to hold incoming data from the client
            while (client.connected()) {                // loop while the client's connected
                if (client.available()) {               // if there's bytes to read from the client,
                    char c = client.read();             // read a byte, then
                    Serial.write(c);                    // print it out the serial monitor
                }
            }
            // close the connection:
            client.stop();
            Serial.println("Client Disconnected.");
        }
    }
}