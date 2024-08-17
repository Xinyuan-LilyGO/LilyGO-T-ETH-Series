#include <Arduino.h>
#if ESP_ARDUINO_VERSION < ESP_ARDUINO_VERSION_VAL(3,0,4) 
#error This sketch can only run on esp32 core version 3.0.0 and above
void setup() {}
void loop() {}
#else
#include <PPP.h>
#include <WiFi.h>
#include "utilities.h"


#define PPP_MODEM_APN "You APN"
#define PPP_MODEM_PIN "0000"  // or NULL

// SIM7600/A7670 basic module with just TX,RX and RST
#define PPP_MODEM_RST     MODEM_PWRKEY_PIN
#define PPP_MODEM_RST_LOW false //active LOW
#define PPP_MODEM_RST_DELAY 200
#define PPP_MODEM_TX      MODEM_TX_PIN
#define PPP_MODEM_RX      MODEM_RX_PIN
#define PPP_MODEM_RTS     -1
#define PPP_MODEM_CTS     -1
#define PPP_MODEM_FC      ESP_MODEM_FLOW_CONTROL_NONE
#define PPP_MODEM_MODEL   PPP_MODEM_SIM7600

// WiFi Access Point Config
#define AP_SSID "ESP32-ETH-WIFI-BRIDGE"
#define AP_PASS "12345678"

IPAddress ap_ip(192, 168, 4, 1);
IPAddress ap_mask(255, 255, 255, 0);
IPAddress ap_leaseStart(192, 168, 4, 2);
IPAddress ap_dns(8, 8, 4, 4);

void setup()
{
    Serial.begin(115200);
    Serial.setDebugOutput(true);

    // Listen for modem events
    Network.onEvent(onEvent);

    // Start the Access Point
    WiFi.AP.begin();
    WiFi.AP.config(ap_ip, ap_ip, ap_mask, ap_leaseStart, ap_dns);
    WiFi.AP.create(AP_SSID, AP_PASS);
    if (!WiFi.AP.waitStatusBits(ESP_NETIF_STARTED_BIT, 1000)) {
        Serial.println("Failed to start AP!");
        return;
    }

    // Configure the modem
    PPP.setApn(PPP_MODEM_APN);
    PPP.setPin(PPP_MODEM_PIN);
    PPP.setResetPin(PPP_MODEM_RST, PPP_MODEM_RST_LOW, PPP_MODEM_RST_DELAY);
    PPP.setPins(PPP_MODEM_TX, PPP_MODEM_RX, PPP_MODEM_RTS, PPP_MODEM_CTS, PPP_MODEM_FC);

    Serial.println("Starting the modem. It might take a while!");
    PPP.begin(PPP_MODEM_MODEL);

    Serial.print("Manufacturer: ");
    Serial.println(PPP.cmd("AT+CGMI", 10000));
    Serial.print("Model: ");
    Serial.println(PPP.moduleName());
    Serial.print("IMEI: ");
    Serial.println(PPP.IMEI());

    bool attached = PPP.attached();
    if (!attached) {
        int i = 0;
        unsigned int s = millis();
        Serial.print("Waiting to connect to network");
        while (!attached && ((++i) < 600)) {
            Serial.print(".");
            delay(100);
            attached = PPP.attached();
        }
        Serial.print((millis() - s) / 1000.0, 1);
        Serial.println("s");
        attached = PPP.attached();
    }

    Serial.print("Attached: ");
    Serial.println(attached);
    Serial.print("State: ");
    Serial.println(PPP.radioState());
    if (attached) {
        Serial.print("Operator: ");
        Serial.println(PPP.operatorName());
        Serial.print("IMSI: ");
        Serial.println(PPP.IMSI());
        Serial.print("RSSI: ");
        Serial.println(PPP.RSSI());
        int ber = PPP.BER();
        if (ber > 0) {
            Serial.print("BER: ");
            Serial.println(ber);
            Serial.print("NetMode: ");
            Serial.println(PPP.networkMode());
        }

        Serial.println("Switching to data mode...");
        PPP.mode(ESP_MODEM_MODE_CMUX);  // Data and Command mixed mode
        if (!PPP.waitStatusBits(ESP_NETIF_CONNECTED_BIT, 1000)) {
            Serial.println("Failed to connect to internet!");
        } else {
            Serial.println("Connected to internet!");
        }
    } else {
        Serial.println("Failed to connect to network!");
    }
}

void loop()
{
    delay(20000);
}

void onEvent(arduino_event_id_t event, arduino_event_info_t info)
{
    switch (event) {
    case ARDUINO_EVENT_PPP_START:     Serial.println("PPP Started"); break;
    case ARDUINO_EVENT_PPP_CONNECTED: Serial.println("PPP Connected"); break;
    case ARDUINO_EVENT_PPP_GOT_IP:
        Serial.println("PPP Got IP");
        Serial.println(PPP);
        WiFi.AP.enableNAPT(true);
        break;
    case ARDUINO_EVENT_PPP_LOST_IP:
        Serial.println("PPP Lost IP");
        WiFi.AP.enableNAPT(false);
        break;
    case ARDUINO_EVENT_PPP_DISCONNECTED:
        Serial.println("PPP Disconnected");
        WiFi.AP.enableNAPT(false);
        break;
    case ARDUINO_EVENT_PPP_STOP: Serial.println("PPP Stopped"); break;

    case ARDUINO_EVENT_WIFI_AP_START:
        Serial.println("AP Started");
        Serial.println(WiFi.AP);
        break;
    case ARDUINO_EVENT_WIFI_AP_STACONNECTED:    Serial.println("AP STA Connected"); break;
    case ARDUINO_EVENT_WIFI_AP_STADISCONNECTED: Serial.println("AP STA Disconnected"); break;
    case ARDUINO_EVENT_WIFI_AP_STAIPASSIGNED:
        Serial.print("AP STA IP Assigned: ");
        Serial.println(IPAddress(info.wifi_ap_staipassigned.ip.addr));
        break;
    case ARDUINO_EVENT_WIFI_AP_PROBEREQRECVED: Serial.println("AP Probe Request Received"); break;
    case ARDUINO_EVENT_WIFI_AP_STOP:           Serial.println("AP Stopped"); break;

    default: break;
    }
}
#endif

