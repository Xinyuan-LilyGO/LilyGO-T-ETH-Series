/**
 * @file      TCPClient.ino
 * @author    Lewis He (lewishe@outlook.com)
 * @license   MIT
 * @copyright Copyright (c) 2023  Shenzhen Xin Yuan Electronic Technology Co., Ltd
 * @date      2023-06-08
 *
 */
#include <Arduino.h>
#include <ETH.h>

#undef ETH_CLK_MODE
#define ETH_CLK_MODE        ETH_CLOCK_GPIO17_OUT
#define ETH_POWER_PIN       5
#define ETH_TYPE            ETH_PHY_LAN8720
#define ETH_ADDR            0
#define ETH_MDC_PIN         23
#define ETH_MDIO_PIN        18

// Connect to the server's IP
const char *host = "192.168.36.76";
// Connect to the server's Port
const int httpPort = 8888;

WiFiClient client;
static bool eth_connected = false;
uint32_t sendMillis = 0;

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

    ETH.begin(ETH_ADDR, ETH_POWER_PIN, ETH_MDC_PIN,
              ETH_MDIO_PIN, ETH_TYPE, ETH_CLK_MODE);

    Serial.println("Connect ti server..");
    while (!client.connect(host, httpPort)) {
        Serial.print("."); delay(500);
    }
    Serial.println();
    Serial.println("Server connected!");
}

void loop()
{

    if (!client.connected()) {
        Serial.println("Connect ti server..");
        while (!client.connect(host, httpPort)) {
            Serial.print("."); delay(500);
        }
        Serial.println();
        Serial.println("Server connected!");
    }

    // Listen for returned messages
    while (client.available()) {
        Serial.write(client.read());
    }

    // Timestamp sent every second
    if (sendMillis < millis()) {
        client.print("->");
        client.println(millis() / 1000);
        sendMillis = millis() + 1000;
    }
}



