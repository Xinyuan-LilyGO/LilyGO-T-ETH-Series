/*
    This sketch shows how to configure different external or internal clock sources for the Ethernet PHY
*/

#include <ETH.h>
#include <SPI.h>
#include <SD.h>

#define SD_MISO 2
#define SD_MOSI 15
#define SD_SCLK 14
#define SD_CS 13
/*
 * ETH_CLOCK_GPIO0_IN   - default: external clock from crystal oscillator
 * ETH_CLOCK_GPIO0_OUT  - 50MHz clock from internal APLL output on GPIO0 - possibly an inverter is needed for LAN8720
 * ETH_CLOCK_GPIO16_OUT - 50MHz clock from internal APLL output on GPIO16 - possibly an inverter is needed for LAN8720
 * ETH_CLOCK_GPIO17_OUT - 50MHz clock from internal APLL inverted output on GPIO17 - tested with LAN8720
 */
// #define ETH_CLK_MODE    ETH_CLOCK_GPIO0_OUT          // Version with PSRAM
#define ETH_CLK_MODE ETH_CLOCK_GPIO17_OUT // Version with not PSRAM

// Pin# of the enable signal for the external crystal oscillator (-1 to disable for internal APLL source)
#define ETH_POWER_PIN -1

// Type of the Ethernet PHY (LAN8720 or TLK110)
#define ETH_TYPE ETH_PHY_LAN8720

// I²C-address of Ethernet PHY (0 or 1 for LAN8720, 31 for TLK110)
#define ETH_ADDR 0

// Pin# of the I²C clock signal for the Ethernet PHY
#define ETH_MDC_PIN 23

// Pin# of the I²C IO signal for the Ethernet PHY
#define ETH_MDIO_PIN 18

#define NRST 5
static bool eth_connected = false;

void WiFiEvent(WiFiEvent_t event)
{
#if ESP_IDF_VERSION_MAJOR > 3
    switch (event)
    {
    case ARDUINO_EVENT_ETH_START:
        Serial.println("ETH Started");
        // set eth hostname here
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
        if (ETH.fullDuplex())
        {
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
#elif
    switch (event)
    {
    case SYSTEM_EVENT_ETH_START:
        Serial.println("ETH Started");
        // set eth hostname here
        ETH.setHostname("esp32-ethernet");
        break;
    case SYSTEM_EVENT_ETH_CONNECTED:
        Serial.println("ETH Connected");
        break;
    case SYSTEM_EVENT_ETH_GOT_IP:
        Serial.print("ETH MAC: ");
        Serial.print(ETH.macAddress());
        Serial.print(", IPv4: ");
        Serial.print(ETH.localIP());
        if (ETH.fullDuplex())
        {
            Serial.print(", FULL_DUPLEX");
        }
        Serial.print(", ");
        Serial.print(ETH.linkSpeed());
        Serial.println("Mbps");
        eth_connected = true;
        break;
    case SYSTEM_EVENT_ETH_DISCONNECTED:
        Serial.println("ETH Disconnected");
        eth_connected = false;
        break;
    case SYSTEM_EVENT_ETH_STOP:
        Serial.println("ETH Stopped");
        eth_connected = false;
        break;
    default:
        break;
    }
#endif
}

void testClient(const char *host, uint16_t port)
{
    Serial.print("\nconnecting to ");
    Serial.println(host);

    WiFiClient client;
    if (!client.connect(host, port))
    {
        Serial.println("connection failed");
        return;
    }
    client.printf("GET / HTTP/1.1\r\nHost: %s\r\n\r\n", host);
    while (client.connected() && !client.available())
        ;
    while (client.available())
    {
        Serial.write(client.read());
    }

    Serial.println("closing connection\n");
    client.stop();
}

void setup()
{
    Serial.begin(115200);

    WiFi.onEvent(WiFiEvent);

    SPI.begin(SD_SCLK, SD_MISO, SD_MOSI, SD_CS);
    if (!SD.begin(SD_CS))
    {
        Serial.println("SDCard MOUNT FAIL");
    }
    else
    {
        uint32_t cardSize = SD.cardSize() / (1024 * 1024);
        String str = "SDCard Size: " + String(cardSize) + "MB";
        Serial.println(str);
    }

    pinMode(NRST, OUTPUT);
    digitalWrite(NRST, 0);
    delay(200);
    digitalWrite(NRST, 1);
    delay(200);
    digitalWrite(NRST, 0);
    delay(200);
    digitalWrite(NRST, 1);

    ETH.begin(ETH_ADDR, ETH_POWER_PIN, ETH_MDC_PIN, ETH_MDIO_PIN, ETH_TYPE, ETH_CLK_MODE);
}

void loop()
{
    if (eth_connected)
    {
        testClient("baidu.com", 80);
    }
    delay(10000);
}
