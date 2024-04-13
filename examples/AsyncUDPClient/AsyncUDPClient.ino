/**
 * @file      AsyncUDPClient.ino
 * @author    Lewis He (lewishe@outlook.com)
 * @license   MIT
 * @copyright Copyright (c) 2023  Shenzhen Xin Yuan Electronic Technology Co., Ltd
 * @date      2023-12-18
 * @note      If you use ETH-Lite-ESP32S3 you need to look here,
 *            Before use, please read here. You need to comment a certain section of AsyncUDP.cpp to work properly.
 *            https://github.com/Xinyuan-LilyGO/LilyGO-T-ETH-Series/issues/49
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
#include <AsyncUDP.h>
#include <WiFi.h>
#include "utilities.h"          //Board PinMap

//The udp library class
AsyncUDP udp;

//IP address to send UDP data to:
// either use the ip address of the server or
// a network broadcast address
IPAddress udpAddress = IPAddress(192, 168, 36, 172);
const uint16_t udpPort = 1234;


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
    if (eth_connected && !udp.connected()) {
        if (udp.connect(udpAddress, udpPort)) {
            Serial.println("UDP connected");
            udp.onPacket([](AsyncUDPPacket packet) {
                Serial.print("UDP Packet Type: ");
                Serial.print(packet.isBroadcast() ? "Broadcast" : packet.isMulticast() ? "Multicast" : "Unicast");
                Serial.print(", From: ");
                Serial.print(packet.remoteIP());
                Serial.print(":");
                Serial.print(packet.remotePort());
                Serial.print(", To: ");
                Serial.print(packet.localIP());
                Serial.print(":");
                Serial.print(packet.localPort());
                Serial.print(", Length: ");
                Serial.print(packet.length());
                Serial.print(", Data: ");
                Serial.write(packet.data(), packet.length());
                Serial.println();
                //reply to the client
                packet.printf("Got %u bytes of data", packet.length());
            });
            //Send unicast
            udp.print("Hello Server!");
        }

    }
    String packet = "millis = " + String(millis() / 1000) + "\n";
    udp.broadcastTo(packet.c_str(), udpPort, TCPIP_ADAPTER_IF_ETH);
    delay(1000);
}