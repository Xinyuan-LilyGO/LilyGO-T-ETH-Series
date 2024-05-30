/**
 * @file      USB_Camera.ino
 * @author    Lewis He (lewishe@outlook.com)
 * @license   MIT
 * @copyright Copyright (c) 2024  ShenZhen XinYuan Electronic Technology Co., Ltd
 * @date      2024-05-30
 * @note      Not all usb cameras support it, only USB cameras with built-in JPEG output format are supported.
 *            Sketch can only be used for ESP32S3 chips, ESP32 cannot be used
 */

/*
       USB Socket      |     ESP32-S3 Board (e.g T-ETH-LITE-ESP32S3)
        5V     ---------->     5V OP         
        GND    ---------->     GND
        D+     ---------->     20
        D-     ---------->     19
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
#include <USB_STREAM.h>
#include "esp_camera.h"
#include "utilities.h"          //Board PinMap

#ifndef CONFIG_IDF_TARGET_ESP32S3
#error "This sketch will only run on ESP32S3 ..."
#endif


extern void app_httpd_main();

#define BIT0_FRAME_START     (0x01 << 0)
#define BIT1_NEW_FRAME_START (0x01 << 1)
#define BIT2_NEW_FRAME_END   (0x01 << 2)
#define BIT3_SPK_START       (0x01 << 3)
#define BIT4_SPK_RESET       (0x01 << 4)
#define BIT5_NET_CONNECT     (0x01 << 5)

static EventGroupHandle_t s_evt_handle;

static camera_fb_t s_fb = {0};


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
        Serial.print("Mbps");
        Serial.print(", ");
        Serial.print("GatewayIP:");
        Serial.println(ETH.gatewayIP());
        xEventGroupSetBits(s_evt_handle, BIT5_NET_CONNECT);
        break;
    case ARDUINO_EVENT_ETH_DISCONNECTED:
        Serial.println("ETH Disconnected");
        xEventGroupClearBits(s_evt_handle, BIT5_NET_CONNECT);
        break;
    case ARDUINO_EVENT_ETH_STOP:
        Serial.println("ETH Stopped");
        xEventGroupClearBits(s_evt_handle, BIT5_NET_CONNECT);
        break;
    default:
        break;
    }
}

camera_fb_t *esp_camera_fb_get()
{
    xEventGroupSetBits(s_evt_handle, BIT0_FRAME_START);
    xEventGroupWaitBits(s_evt_handle, BIT1_NEW_FRAME_START, true, true, portMAX_DELAY);
    return &s_fb;
}

void esp_camera_fb_return(camera_fb_t *fb)
{
    xEventGroupSetBits(s_evt_handle, BIT2_NEW_FRAME_END);
    return;
}

static void onCameraFrameCallback(uvc_frame_t *frame, void *ptr)
{
    // Serial.printf("uvc callback! frame_format = %d, seq = %"PRIu32", width = %"PRIu32", height = %"PRIu32", length = %u, ptr = %d\n",
    //               frame->frame_format, frame->sequence, frame->width, frame->height, frame->data_bytes, (int) ptr);
    if (!(xEventGroupGetBits(s_evt_handle) & BIT0_FRAME_START)) {
        return;
    }

    switch (frame->frame_format) {
    case UVC_FRAME_FORMAT_MJPEG:
        s_fb.buf = (uint8_t *)frame->data;
        s_fb.len = frame->data_bytes;
        s_fb.width = frame->width;
        s_fb.height = frame->height;
        s_fb.buf = (uint8_t *)frame->data;
        s_fb.format = PIXFORMAT_JPEG;
        s_fb.timestamp.tv_sec = frame->sequence;
        xEventGroupSetBits(s_evt_handle, BIT1_NEW_FRAME_START);
        // Serial.printf("send frame = %"PRIu32"", frame->sequence);
        xEventGroupWaitBits(s_evt_handle, BIT2_NEW_FRAME_END, true, true, portMAX_DELAY);
        // Serial.printf("send frame done = %"PRIu32"", frame->sequence);
        break;
    default:
        ESP_LOGW(TAG, "Format not supported");
        assert(0);
        break;
    }
}

void setup()
{
    Serial.begin(115200);

    s_evt_handle = xEventGroupCreate();
    if (s_evt_handle == NULL) {
        Serial.printf("line-%u event group create failed", __LINE__);
        assert(0);
    }

    // Set WiFi to station mode and disconnect from an AP if it was previously connected
    WiFi.mode(WIFI_STA);
    WiFi.disconnect();
    WiFi.onEvent(WiFiEvent);

    /*
    const char *ssid     = "Your WiFi SSID"; // Change this to your WiFi SSID
    const char *password = "Your WiFi PASSWORD"; // Change this to your WiFi password
    // Run On Wifi
    Serial.println();
    Serial.println("******************************************************");
    Serial.print("Connecting to ");
    Serial.println(ssid);

    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    Serial.println("");
    Serial.println("WiFi connected");
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP());
    */

    // Run On ETH
    if (!ETH.begin(ETH_PHY_W5500, 1, ETH_CS_PIN, ETH_INT_PIN, ETH_RST_PIN,
                   SPI3_HOST,
                   ETH_SCLK_PIN, ETH_MISO_PIN, ETH_MOSI_PIN)) {
        Serial.println("ETH start Failed!");
    }

    // Instantiate an object
    USB_STREAM *usb = new USB_STREAM();

    // allocate memory
    uint8_t *_xferBufferA = (uint8_t *)malloc(55 * 1024);
    assert(_xferBufferA != NULL);
    uint8_t *_xferBufferB = (uint8_t *)malloc(55 * 1024);
    assert(_xferBufferB != NULL);
    uint8_t *_frameBuffer = (uint8_t *)malloc(55 * 1024);
    assert(_frameBuffer != NULL);

    // Config the parameter
    usb->uvcConfiguration(FRAME_RESOLUTION_ANY, FRAME_RESOLUTION_ANY, FRAME_INTERVAL_FPS_15, 55 * 1024, _xferBufferA, _xferBufferB, 55 * 1024, _frameBuffer);

    //Register the camera frame callback function
    usb->uvcCamRegisterCb(&onCameraFrameCallback, NULL);

    usb->start();

    usb->connectWait(1000);

    usb->uvcCamSuspend(NULL);

    delay(5000);

    // Waiting for network access
    xEventGroupWaitBits(s_evt_handle, BIT5_NET_CONNECT, true, false, portMAX_DELAY);

    app_httpd_main();

    usb->uvcCamResume(NULL);

    /*Dont forget to free the allocated memory*/
    // free(_xferBufferA);
    // free(_xferBufferB);
    // free(_frameBuffer);
}

void loop()
{
    Serial.println(millis());
    vTaskDelay(10000);
}
