[![Arduino Lint](https://github.com/esp-arduino-libs/ESP32_USB_Stream/actions/workflows/arduino_lint.yml/badge.svg)](https://github.com/esp-arduino-libs/ESP32_USB_Stream/actions/workflows/arduino_lint.yml) [![pre-commit](https://github.com/esp-arduino-libs/ESP32_USB_Stream/actions/workflows/pre-commit.yml/badge.svg)](https://github.com/esp-arduino-libs/ESP32_USB_Stream/actions/workflows/pre-commit.yml)

# ESP32_USB_STREAM

ESP32_USB_STREAM is an Arduino library designed to support USB UVC + UAC host driver for ESP32-S2/ESP32-S3. It supports read/write/control multimedia streaming from usb device. For example, at most one UVC + one Microphone + one Speaker streaming can be supported at the same time.

ESP32_USB_STREAM encapsulates the component from the [Espressif Components Registry](https://components.espressif.com/). It is developed based on [arduino-esp32](https://github.com/espressif/arduino-esp32) and can be easily downloaded and integrated into the Arduino IDE.

## Features

* Only support for ESP32-S2 and ESP32-S3 SoCs.
* Support video stream through UVC Stream interface.
* Support microphone stream and speaker stream through the UAC Stream interface
* Support volume, mute and other features control through the UAC Control interface
* Support stream separately suspend and resume

## Supported Drivers

|                             **Driver**                             | **Version** |
| ------------------------------------------------------------------ | ----------- |
| [usb_stream](https://components.espressif.com/components/espressif/usb_stream) |1.2.0|

## How to Use

For information on how to use the library in the Arduino IDE, please refer to the documentation for [Arduino IDE v1.x.x](https://docs.arduino.cc/software/ide-v1/tutorials/installing-libraries) or [Arduino IDE v2.x.x](https://docs.arduino.cc/software/ide-v2/tutorials/ide-v2-installing-a-library).

## Dependencies Version

|                                  **Name**                                  | **Version** |
| -------------------------------------------------------------------------- | ----------- |
| [arduino-esp32](https://github.com/espressif/arduino-esp32)                | >= v2.0.14  |

### Examples

* [Getting started with a UVC](examples/GettingStartUVC/): Demonstrates how to use usb video streaming.
* [Getting started with a UAC](examples/GettingStartUAC/): Demonstrates how to use usb audio streaming.

### Detailed Usage

```cpp
#include "USB_STREAM.h"

// Instantiate a Ustream object
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
usb->uvcCamRegisterFrameCb(&cameraFramecb, NULL);

usb->start();

/*Dont forget to free the allocated memory*/
// free(_xferBufferA);
// free(_xferBufferB);
// free(_frameBuffer);

```
Note: For additional details and information about the **usb_stream** functionality, please refer to the documentation provided by [ESP-IOT Solutions](https://github.com/espressif/esp-iot-solution/tree/master/components/usb/usb_stream).