#include <Arduino.h>
#include "USB_STREAM.h"

/* Define the camera frame callback function implementation */
static void onCameraFrameCallback(uvc_frame *frame, void *user_ptr)
{
    Serial.printf("uvc callback! frame_format = %d, seq = %" PRIu32 ", width = %" PRIu32", height = %" PRIu32 ", length = %u, ptr = %d\n",
             frame->frame_format, frame->sequence, frame->width, frame->height, frame->data_bytes, (int)user_ptr);
}

void setup()
{
    Serial.begin(115200);
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
    delay(5000);

    usb->uvcCamSuspend(NULL);
    delay(5000);

    usb->uvcCamResume(NULL);

    /*Dont forget to free the allocated memory*/
    // free(_xferBufferA);
    // free(_xferBufferB);
    // free(_frameBuffer);
}

void loop()
{
    vTaskDelay(100);
}
