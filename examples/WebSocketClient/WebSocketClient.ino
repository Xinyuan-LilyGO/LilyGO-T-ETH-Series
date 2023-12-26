/**
 * @file      WebSocketClient.ino
 * @author    Lewis He (lewishe@outlook.com)
 * @license   MIT
 * @copyright Copyright (c) 2023  Shenzhen Xin Yuan Electronic Technology Co., Ltd
 * @date      2023-12-25
 * @note      API docs: https://docs.espressif.com/projects/esp-protocols/esp_websocket_client/docs/latest/index.html
 */
#include <Arduino.h>
// #include <ETH.h>
#include <ETHClass.h>       //Is to use the modified ETHClass
#include <SPI.h>
#include <SD.h>
#include "utilities.h"          //Board PinMap
#include "esp_websocket_client.h"
#include "freertos/semphr.h"

#define NO_DATA_TIMEOUT_SEC 5

static TimerHandle_t shutdown_signal_timer;
static SemaphoreHandle_t shutdown_sema;

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

void shutdown_signaler(TimerHandle_t xTimer)
{
    Serial.printf( "No data received for %d seconds, signaling shutdown\n", NO_DATA_TIMEOUT_SEC);
    xSemaphoreGive(shutdown_sema);
}

void websocket_event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data)
{
    esp_websocket_event_data_t *data = (esp_websocket_event_data_t *)event_data;
    switch (event_id) {
    case WEBSOCKET_EVENT_CONNECTED:
        Serial.println( "WEBSOCKET_EVENT_CONNECTED");
        break;
    case WEBSOCKET_EVENT_DISCONNECTED:
        Serial.println( "WEBSOCKET_EVENT_DISCONNECTED");
        break;
    case WEBSOCKET_EVENT_DATA:
        Serial.println( "WEBSOCKET_EVENT_DATA");
        Serial.printf( "Received opcode=%d\n", data->op_code);
        if (data->op_code == 0x08 && data->data_len == 2) {
            Serial.printf("Received closed message with code=%d\n", 256 * data->data_ptr[0] + data->data_ptr[1]);
        } else {
            Serial.printf("Received=%.*s", data->data_len, (char *)data->data_ptr);
        }
        Serial.printf( "Total payload length=%d, data_len=%d, current payload offset=%d\n", data->payload_len, data->data_len, data->payload_offset);
        xTimerReset(shutdown_signal_timer, portMAX_DELAY);
        break;
    case WEBSOCKET_EVENT_ERROR:
        Serial.println( "WEBSOCKET_EVENT_ERROR");
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
    if (!ETH.begin(ETH_ADDR, ETH_RESET_PIN, ETH_MDC_PIN,
                   ETH_MDIO_PIN, ETH_TYPE, ETH_CLK_MODE)) {
        Serial.println("ETH start Failed!");
    }
#else
    if (!ETH.beginSPI(ETH_MISO_PIN, ETH_MOSI_PIN, ETH_SCLK_PIN, ETH_CS_PIN, ETH_RST_PIN, ETH_INT_PIN)) {
        Serial.println("ETH start Failed!");
    }
#endif

    while (!eth_connected) {
        Serial.println("Wait eth cable connect..."); delay(1000);
    }

    esp_websocket_client_config_t websocket_cfg = {};

    shutdown_signal_timer = xTimerCreate("Websocket shutdown timer", NO_DATA_TIMEOUT_SEC * 1000 / portTICK_PERIOD_MS,
                                         pdFALSE, NULL, shutdown_signaler);
    shutdown_sema = xSemaphoreCreateBinary();

    websocket_cfg.uri = "ws://192.168.36.172";

    Serial.printf( "Connecting to %s...\n", websocket_cfg.uri);
    esp_websocket_client_handle_t client = esp_websocket_client_init(&websocket_cfg);
    esp_websocket_register_events(client, WEBSOCKET_EVENT_ANY, websocket_event_handler, (void *)client);

    esp_websocket_client_start(client);
    xTimerStart(shutdown_signal_timer, portMAX_DELAY);
    char data[32];
    int i = 0;
    while (i < 5) {
        if (esp_websocket_client_is_connected(client)) {
            int len = sprintf(data, "hello %04d", i++);
            Serial.printf( "Sending %s\n", data);
            esp_websocket_client_send_text(client, data, len, portMAX_DELAY);
        }
        delay(1000);
    }

    xSemaphoreTake(shutdown_sema, portMAX_DELAY);
    esp_websocket_client_close(client, portMAX_DELAY);
    Serial.println( "Websocket Stopped");
    esp_websocket_client_destroy(client);
}

void loop()
{
}
