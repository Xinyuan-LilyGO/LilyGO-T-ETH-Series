/**
 * @file      MQTTClient.ino
 * @author    Lewis He (lewishe@outlook.com)
 * @license   MIT
 * @copyright Copyright (c) 2023  Shenzhen Xin Yuan Electronic Technology Co., Ltd
 * @date      2023-03-29
 *
 */


#include <SPI.h>
#include <PubSubClient.h>
#include <ETH.h>

#define ETH_CLK_MODE ETH_CLOCK_GPIO17_OUT

// Pin# of the enable signal for the external crystal oscillator (-1 to disable for internal APLL source)
#define ETH_POWER_PIN -1

// Type of the Ethernet PHY (LAN8720 or TLK110)
#define ETH_TYPE      ETH_PHY_LAN8720

// I²C-address of Ethernet PHY (0 or 1 for LAN8720, 31 for TLK110)
#define ETH_ADDR      0

// Pin# of the I²C clock signal for the Ethernet PHY
#define ETH_MDC_PIN   23

// Pin# of the I²C IO signal for the Ethernet PHY
#define ETH_MDIO_PIN  18
#define NRST          5


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

    pinMode(NRST, OUTPUT);
    digitalWrite(NRST, 0);
    delay(200);
    digitalWrite(NRST, 1);
    delay(200);
    digitalWrite(NRST, 0);
    delay(200);
    digitalWrite(NRST, 1);


    ETH.begin(ETH_ADDR, ETH_POWER_PIN, ETH_MDC_PIN,
              ETH_MDIO_PIN, ETH_TYPE, ETH_CLK_MODE);
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
