/**
 * @file      InternetComTest.ino
 * @author    Lewis He (lewishe@outlook.com)
 * @license   MIT
 * @copyright Copyright (c) 2023  Shenzhen Xin Yuan Electronic Technology Co., Ltd
 * @date      2024-01-17
 * @note      Applies to T-Internet-Com only, for factory test hardware only
 */
#include <Arduino.h>
#include <WiFi.h>

#if ESP_ARDUINO_VERSION < ESP_ARDUINO_VERSION_VAL(3,0,0)
#include <ETHClass2.h>       //Is to use the modified ETHClass
#define ETH  ETH2
#include <WiFiClientSecure.h>
WiFiClientSecure clientSecure;
#else
#include <ETH.h>
#include <NetworkClientSecure.h>
NetworkClientSecure clientSecure;
#endif
#include <SPI.h>
#include <SD.h>
#include <time.h>
#include <esp_sntp.h>
#include "utilities.h"          //Board PinMap
#include "HTTPClient.h"
#include "Adafruit_NeoPixel.h"

const char *rootCACertificate =
    "-----BEGIN CERTIFICATE-----\r\n"
    "MIIDzTCCArWgAwIBAgIQCjeHZF5ftIwiTv0b7RQMPDANBgkqhkiG9w0BAQsFADBa\r\n"
    "MQswCQYDVQQGEwJJRTESMBAGA1UEChMJQmFsdGltb3JlMRMwEQYDVQQLEwpDeWJl\r\n"
    "clRydXN0MSIwIAYDVQQDExlCYWx0aW1vcmUgQ3liZXJUcnVzdCBSb290MB4XDTIw\r\n"
    "MDEyNzEyNDgwOFoXDTI0MTIzMTIzNTk1OVowSjELMAkGA1UEBhMCVVMxGTAXBgNV\r\n"
    "BAoTEENsb3VkZmxhcmUsIEluYy4xIDAeBgNVBAMTF0Nsb3VkZmxhcmUgSW5jIEVD\r\n"
    "QyBDQS0zMFkwEwYHKoZIzj0CAQYIKoZIzj0DAQcDQgAEua1NZpkUC0bsH4HRKlAe\r\n"
    "nQMVLzQSfS2WuIg4m4Vfj7+7Te9hRsTJc9QkT+DuHM5ss1FxL2ruTAUJd9NyYqSb\r\n"
    "16OCAWgwggFkMB0GA1UdDgQWBBSlzjfq67B1DpRniLRF+tkkEIeWHzAfBgNVHSME\r\n"
    "GDAWgBTlnVkwgkdYzKz6CFQ2hns6tQRN8DAOBgNVHQ8BAf8EBAMCAYYwHQYDVR0l\r\n"
    "BBYwFAYIKwYBBQUHAwEGCCsGAQUFBwMCMBIGA1UdEwEB/wQIMAYBAf8CAQAwNAYI\r\n"
    "KwYBBQUHAQEEKDAmMCQGCCsGAQUFBzABhhhodHRwOi8vb2NzcC5kaWdpY2VydC5j\r\n"
    "b20wOgYDVR0fBDMwMTAvoC2gK4YpaHR0cDovL2NybDMuZGlnaWNlcnQuY29tL09t\r\n"
    "bmlyb290MjAyNS5jcmwwbQYDVR0gBGYwZDA3BglghkgBhv1sAQEwKjAoBggrBgEF\r\n"
    "BQcCARYcaHR0cHM6Ly93d3cuZGlnaWNlcnQuY29tL0NQUzALBglghkgBhv1sAQIw\r\n"
    "CAYGZ4EMAQIBMAgGBmeBDAECAjAIBgZngQwBAgMwDQYJKoZIhvcNAQELBQADggEB\r\n"
    "AAUkHd0bsCrrmNaF4zlNXmtXnYJX/OvoMaJXkGUFvhZEOFp3ArnPEELG4ZKk40Un\r\n"
    "+ABHLGioVplTVI+tnkDB0A+21w0LOEhsUCxJkAZbZB2LzEgwLt4I4ptJIsCSDBFe\r\n"
    "lpKU1fwg3FZs5ZKTv3ocwDfjhUkV+ivhdDkYD7fa86JXWGBPzI6UAPxGezQxPk1H\r\n"
    "goE6y/SJXQ7vTQ1unBuCJN0yJV0ReFEQPaA1IwQvZW+cwdFD19Ae8zFnWSfda9J1\r\n"
    "CZMRJCQUzym+5iPDuI9yP+kHyCREU3qzuWFloUwOxkgAyXVjBYdwRVKD05WdRerw\r\n"
    "6DEdfgkfCv4+3ao8XnTSrLE=\r\n"
    "-----END CERTIFICATE-----\r\n";


#define SerialMon   Serial
#define SerialAT    Serial1
#define TINY_GSM_MODEM_SIM7600
#include <TinyGsmClient.h>

// #define DUMP_AT_COMMANDS
#ifdef DUMP_AT_COMMANDS
#include <StreamDebugger.h>
StreamDebugger debugger(SerialAT, SerialMon);
TinyGsm        modem(debugger);
#else
TinyGsm        modem(SerialAT);
#endif

Adafruit_NeoPixel pixels(1, RGBLED_PIN, NEO_GRB + NEO_KHZ800);


#ifndef WIFI_SSID
#define WIFI_SSID             "Your WiFi SSID"
#endif
#ifndef WIFI_PASSWORD
#define WIFI_PASSWORD         "Your WiFi PASSWORD"
#endif

static bool wifi_connected = false;
static bool eth_connected = false;
const char *ntpServer1 = "pool.ntp.org";
const char *ntpServer2 = "time.nist.gov";
const long  gmtOffset_sec = 3600;
const int   daylightOffset_sec = 3600;
const char *time_zone = "CET-1CEST,M3.5.0,M10.5.0/3";  // TimeZone rule for Europe/Rome including daylight adjustment rules (optional)
uint32_t intervalue = 0, runWiFi = 0, runRS485 = 0;

void pci_express_task(void *args);

void printLocalTime()
{
    struct tm timeinfo;
    if (!getLocalTime(&timeinfo)) {
        return;
    }
    Serial.print(&timeinfo, "%Y %H:%M:%S");
}

// Callback function (get's called when time adjusts via NTP)
void timeavailable(struct timeval *t)
{
    // printLocalTime();
}

void WiFiEvent(arduino_event_id_t event)
{
    switch (event) {
    case ARDUINO_EVENT_WIFI_STA_LOST_IP:
    // Serial.println("Lost IP address and IP address is reset to 0");
    case ARDUINO_EVENT_WIFI_STA_DISCONNECTED:
        // Serial.println("Disconnected from WiFi access point");
        wifi_connected = false;
        break;
    case ARDUINO_EVENT_WIFI_STA_GOT_IP:
        // Serial.print("Obtained IP address: ");
        // Serial.println(WiFi.localIP());
        wifi_connected = true;
        break;
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
        Serial.print("Mbps");
        Serial.print(", ");
        Serial.print("GatewayIP:");
        Serial.println(ETH.gatewayIP());
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

void testClient(const char *host, uint16_t port)
{
    Serial.print("\nconnecting to ");
    Serial.println(host);

    WiFiClient client;
    if (!client.connect(host, port)) {
        Serial.println("connection failed");
        return;
    }
    client.printf("GET / HTTP/1.1\r\nHost: %s\r\n\r\n", host);
    while (client.connected() && !client.available())
        ;
    while (client.available()) {
        Serial.write(client.read());
    }

    Serial.println("closing connection\n");
    client.stop();
}


void testHTTPS()
{
    const char *host = "https://ipapi.co/json/";
    clientSecure.setCACert(rootCACertificate);
    HTTPClient https;
    if (https.begin(clientSecure, host)) {
        int httpCode = https.GET();
        if (httpCode > 0) {
            // HTTP header has been send and Server response header has been handled
            // Serial.printf("[HTTPS] GET... code: %d\n", httpCode);
            // file found at server
            if (httpCode == HTTP_CODE_OK || httpCode == HTTP_CODE_MOVED_PERMANENTLY) {
                // String payload = https.getString();
                // Serial.println(payload);
                printLocalTime();
                Serial.printf(" > Connect %s  successed!\n", host);
            }
        } else {
            printLocalTime();
            Serial.printf(" > Connect %s  failed err:%s!\n", host, https.errorToString(httpCode).c_str());
        }
        https.end();
    }
}

void setup()
{
    Serial.begin(115200);

    SerialAT.begin(115200, SERIAL_8N1, MODEM_RX_PIN, MODEM_TX_PIN);

    WiFi.onEvent(WiFiEvent);

    Serial.println("===============Pixels =============");
    pixels.begin();
    pixels.setBrightness(50);
    pixels.setPixelColor(0, pixels.Color(255,   0,   0));
    Serial.println("R");
    pixels.show();
    delay(1000);
    pixels.setPixelColor(0, pixels.Color(0,   255,   0));
    pixels.show();
    Serial.println("G");
    delay(1000);
    pixels.setPixelColor(0, pixels.Color(0,   0,   255));
    pixels.show();
    Serial.println("B");
    delay(1000);
    pixels.clear();
    pixels.show();
    Serial.println("===================================\n");


#ifdef ETH_POWER_PIN
    pinMode(ETH_POWER_PIN, OUTPUT);
    digitalWrite(ETH_POWER_PIN, HIGH);
#endif

    /*
    // Use static ip address config
    IPAddress local_ip(192, 168, 50, 222);
    IPAddress gateway(192, 168, 50, 1);
    IPAddress subnet(255, 255, 255, 0);

    ETH.config( local_ip,
                gateway,
                subnet
                // IPAddress dns1 = (uint32_t)0x00000000,
                // IPAddress dns2 = (uint32_t)0x00000000
              );
    */

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



    // set notification call-back function
    sntp_set_time_sync_notification_cb( timeavailable );

    /**
     * NTP server address could be aquired via DHCP,
     *
     * NOTE: This call should be made BEFORE esp32 aquires IP address via DHCP,
     * otherwise SNTP option 42 would be rejected by default.
     * NOTE: configTime() function call if made AFTER DHCP-client run
     * will OVERRIDE aquired NTP server address
     */
    sntp_servermode_dhcp(1);    // (optional)

    /**
     * This will set configured ntp servers and constant TimeZone/daylightOffset
     * should be OK if your time zone does not need to adjust daylightOffset twice a year,
     * in such a case time adjustment won't be handled automagicaly.
     */
    configTime(gmtOffset_sec, daylightOffset_sec, ntpServer1, ntpServer2);


    // Set WiFi to station mode and disconnect from an AP if it was previously connected
    WiFi.mode(WIFI_STA);
    WiFi.disconnect();
    delay(100);


    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

#ifdef SD_MISO_PIN
    pinMode(SD_MISO_PIN, INPUT_PULLUP);
    SPI.begin(SD_SCLK_PIN, SD_MISO_PIN, SD_MOSI_PIN, SD_CS_PIN);

    SD.begin(SD_CS_PIN);

    Serial.println("===========SD Card Result==========");

    if (SD.cardType() == CARD_UNKNOWN || SD.cardType() == CARD_NONE) {
        Serial.println("!!!!!ERROR : Not detect SDMMC !!!!!");
    } else {
        uint32_t cardSize = SD.cardSize() / (1024 * 1024);
        Serial.print(" > size:");
        Serial.print(cardSize);
        Serial.println(" MB");
    }
    Serial.println("===================================\n");
#endif


    xTaskCreate(pci_express_task, "pcie", 4 * 1024, NULL, 10, NULL);

}


bool runOnETH = false;

void loop()
{
    if (millis() - intervalue >= 10000) {
        if (runOnETH) {
            if (eth_connected) {
                Serial.println("===========RUN ON ETH==============");
                //test http
                // testClient("httpbin.org", 80);
                testHTTPS();
                printLocalTime();
                Serial.print(" > ETH IP Address:");
                Serial.println(ETH.localIP());
            } else {
                Serial.println("!!!!!!ETH  NOT CONNECTED!!!!!!!!");
            }
        } else {
            Serial.println("===========RUN ON WiFi=============");

            if (wifi_connected) {
                printLocalTime();
                Serial.print(" > WIFI IP Address:");
                Serial.println(WiFi.localIP());
            } else {
                Serial.println("!!!!!!WIFI NOT CONNECTED!!!!!!!!");
            }
        }
        Serial.println("===================================\n");

        if (runOnETH) {
            pixels.setPixelColor(0, pixels.Color(255,   255,   255));
        } else {
            pixels.clear();
        }
        pixels.show();
        runOnETH ^=  1;
        intervalue = millis();
    }

    delay(10);
}


void pci_express_task(void *args)
{
    String result;
    SimStatus  status;
    // Pull down PWRKEY for more than 1 second according to manual requirements
    digitalWrite(MODEM_PWRKEY_PIN, LOW);
    delay(100);
    digitalWrite(MODEM_PWRKEY_PIN, HIGH);
    delay(1000);
    digitalWrite(MODEM_PWRKEY_PIN, LOW);

    while (1) {

        int retry = 0;

        while (!modem.testAT(5000)) {
            // Pull down PWRKEY for more than 1 second according to manual requirements
            digitalWrite(MODEM_PWRKEY_PIN, LOW);
            delay(100);
            digitalWrite(MODEM_PWRKEY_PIN, HIGH);
            delay(1000);
            digitalWrite(MODEM_PWRKEY_PIN, LOW);
            if (retry++ > 3) {
                result = (" > !!!!!ERROR : Modem not response, delete task!!!!!");
                goto ERR0;
            }
        }

        retry = 0;
        while ((status = modem.getSimStatus()) != SIM_READY && retry++ > 6) {
            delay(10);
        }
        if (status != SIM_READY) {
            result = (" > !!!!!ERROR : Modem not detected sim card, delete task!!!!!");
            goto ERR0;
        }

        Serial.println("========PCI EXPRESS Result=========");
        printLocalTime();
        Serial.println(" > Modem has poweron successed and sim status ok !");
        Serial.println("===================================\n");

        vTaskDelete(NULL);
        return ;
ERR0:
        Serial.println("===========PCI EXPRESS Result======");
        printLocalTime();
        Serial.println(result);
        Serial.println("===================================\n");
        vTaskDelete(NULL);
    }
}