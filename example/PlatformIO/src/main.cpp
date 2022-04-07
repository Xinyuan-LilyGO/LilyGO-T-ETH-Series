#include <Arduino.h>
#include <ETH.h>
#include <SPI.h>
#include "Config/config.h"
#include <HTTPClient.h>
static bool eth_connected = false;
static bool eth_done = false;
void WiFiEvent(WiFiEvent_t event)
{
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

void ETH_test(void)
{
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

void setup()
{
  Serial.begin(115200);

  pinMode(ETH_POWER_PIN, OUTPUT);
  digitalWrite(ETH_POWER_PIN, 1);

  WiFi.onEvent(WiFiEvent);
  ETH_test();
}

void loop()
{

  if (eth_connected && !eth_done)
  {
    testClient("baidu.com", 80);
    eth_done = true;
  }
}
