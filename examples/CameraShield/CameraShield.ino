/**
 * @file      CameraShield.ino
 * @author    Lewis He (lewishe@outlook.com)
 * @license   MIT
 * @copyright Copyright (c) 2023  Shenzhen Xin Yuan Electronic Technology Co., Ltd
 * @date      2023-08-07
 * @note      Only support T-ETH-Lite-ESP32S3, need external camera shield to combine
 */

#include "esp_camera.h"
#include <WiFi.h>
// #include <ETH.h>
#include <ETHClass.h>       //Is to use the modified ETHClass
#include "utilities.h"          //Board PinMap
//
// WARNING!!!
//            You must select partition scheme from the board menu that has at least 3MB APP space.
//            Only support T-ETH-Lite-ESP32S3, need external camera shield to combine

//camera Shield pinmap
#define PWDN_GPIO_NUM    -1
#define RESET_GPIO_NUM   47
#define XCLK_GPIO_NUM    45
#define SIOD_GPIO_NUM    38
#define SIOC_GPIO_NUM    39

#define Y9_GPIO_NUM      41
#define Y8_GPIO_NUM      2
#define Y7_GPIO_NUM      1
#define Y6_GPIO_NUM      16
#define Y5_GPIO_NUM      18
#define Y4_GPIO_NUM      3
#define Y3_GPIO_NUM      8
#define Y2_GPIO_NUM      17
#define VSYNC_GPIO_NUM   40
#define HREF_GPIO_NUM    48
#define PCLK_GPIO_NUM    15
// Define in <utilities.h>
// #define IR_FILTER_NUM    46


void startCameraServer();


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

void setup()
{
    Serial.begin(115200);
    Serial.setDebugOutput(true);
    Serial.println();

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

    // The infrared filter function is configured through the web
    pinMode(IR_FILTER_NUM, OUTPUT);

    while (!eth_connected) {
        Serial.println("Wait ETH Connect...");
        delay(1000);
    };


    camera_config_t config;
    config.ledc_channel = LEDC_CHANNEL_0;
    config.ledc_timer = LEDC_TIMER_0;
    config.pin_d0 = Y2_GPIO_NUM;
    config.pin_d1 = Y3_GPIO_NUM;
    config.pin_d2 = Y4_GPIO_NUM;
    config.pin_d3 = Y5_GPIO_NUM;
    config.pin_d4 = Y6_GPIO_NUM;
    config.pin_d5 = Y7_GPIO_NUM;
    config.pin_d6 = Y8_GPIO_NUM;
    config.pin_d7 = Y9_GPIO_NUM;
    config.pin_xclk = XCLK_GPIO_NUM;
    config.pin_pclk = PCLK_GPIO_NUM;
    config.pin_vsync = VSYNC_GPIO_NUM;
    config.pin_href = HREF_GPIO_NUM;
    config.pin_sccb_sda = SIOD_GPIO_NUM;
    config.pin_sccb_scl = SIOC_GPIO_NUM;
    config.pin_pwdn = PWDN_GPIO_NUM;
    config.pin_reset = RESET_GPIO_NUM;
    config.xclk_freq_hz = 20000000;
    config.frame_size = FRAMESIZE_UXGA;
    config.pixel_format = PIXFORMAT_JPEG; // for streaming
    //config.pixel_format = PIXFORMAT_RGB565; // for face detection/recognition
    config.grab_mode = CAMERA_GRAB_WHEN_EMPTY;
    config.fb_location = CAMERA_FB_IN_PSRAM;
    config.jpeg_quality = 12;
    config.fb_count = 1;

    // if PSRAM IC present, init with UXGA resolution and higher JPEG quality
    //                      for larger pre-allocated frame buffer.
    if (config.pixel_format == PIXFORMAT_JPEG) {
        if (psramFound()) {
            config.jpeg_quality = 10;
            config.fb_count = 2;
            config.grab_mode = CAMERA_GRAB_LATEST;
        } else {
            // Limit the frame size when PSRAM is not available
            config.frame_size = FRAMESIZE_SVGA;
            config.fb_location = CAMERA_FB_IN_DRAM;
        }
    } else {
        // Best option for face detection/recognition
        config.frame_size = FRAMESIZE_240X240;
        config.fb_count = 2;
    }

    // camera init
    esp_err_t err = esp_camera_init(&config);
    if (err != ESP_OK) {
        Serial.printf("Camera init failed with error 0x%x", err);
        return;
    }

    Serial.println("Camera Start!!!");
    sensor_t *s = esp_camera_sensor_get();
    // initial sensors are flipped vertically and colors are a bit saturated
    if (s->id.PID == OV5640_PID) {
        s->set_vflip(s, 1); // flip it back
        // s->set_brightness(s, 1); // up the brightness just a bit
        // s->set_saturation(s, -2); // lower the saturation
    } else {
        s->set_vflip(s, 0);
    }
    // drop down frame size for higher initial frame rate
    if (config.pixel_format == PIXFORMAT_JPEG) {
        s->set_framesize(s, FRAMESIZE_QVGA);
    }


    startCameraServer();

}

void loop()
{
    // Do nothing. Everything is done in another task by the web server
    delay(10000);
}
