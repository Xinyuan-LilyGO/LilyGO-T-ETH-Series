/**
 * @file      MQTTClient.ino
 * @author    Lewis He (lewishe@outlook.com)
 * @license   MIT
 * @copyright Copyright (c) 2023  Shenzhen Xin Yuan Electronic Technology Co., Ltd
 * @date      2023-03-29
 *
 */
// #include <ETH.h>
#include <ETHClass.h>       //Is to use the modified ETHClass
#include <PubSubClient.h>
#include <SPI.h>
#include "utilities.h"          //Board PinMap


bool eth_connected = false;

WiFiClient ethClient;
PubSubClient client(ethClient);

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

void callback(char *topic, uint8_t *payload, uint32_t length)
{
    Serial.print("Message arrived [");
    Serial.print(topic);
    Serial.print("] ");
    for (int i = 0; i < length; i++) {
        Serial.print((char)payload[i]);
    }
    Serial.println();
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
    if (!ETH.begin(ETH_ADDR, ETH_RESET_PIN, ETH_MDC_PIN,
                   ETH_MDIO_PIN, ETH_TYPE, ETH_CLK_MODE)) {
        Serial.println("ETH start Failed!");
    }
#else
    if (!ETH.beginSPI(ETH_MISO_PIN, ETH_MOSI_PIN, ETH_SCLK_PIN, ETH_CS_PIN, ETH_RST_PIN, ETH_INT_PIN)) {
        Serial.println("ETH start Failed!");
    }
#endif
    // Note - the default maximum packet size is 128 bytes. If the
    // combined length of clientId, username and password exceed this use the
    // following to increase the buffer size:
    // client.setBufferSize(255);
    while (!eth_connected) {
        Serial.println("Wait eth connect..."); delay(2000);
    }

    //set server and port
    client.setServer("broker-cn.emqx.io", 1883);


    //set callback
    client.setCallback(callback);


    const char *clien_id = "esp32eth";
    const char *username = "empty";
    const char *password = "empty";

    while (!client.connect(clien_id, username, password)) {
        delay(200);
        Serial.println("Connecting....");
    }

    client.subscribe("/esp32eth/led");

    // publish example
    // client.publish("outTopic", "hello world");
    Serial.println("MQTT Connected!");
}

void loop()
{
    client.loop();
}
