#include <Arduino.h>
#include "USB_STREAM.h"

/* Define the Mic frame callback function implementation */
static void onMicFrameCallback(mic_frame_t *frame, void *ptr)
{
    // We should using higher baudrate here, to reduce the blocking time here
    Serial.printf("mic callback! bit_resolution = %u, samples_frequence = %"PRIu32", data_bytes = %"PRIu32"\n", frame->bit_resolution, frame->samples_frequence, frame->data_bytes);
}

void setup()
{
    Serial.begin(115200);
    // Instantiate a Ustream object
    USB_STREAM *usb = new USB_STREAM();

    // Config the parameter
    usb->uacConfiguration(UAC_CH_ANY, UAC_BITS_ANY, UAC_FREQUENCY_ANY, 6400, UAC_CH_ANY, UAC_BITS_ANY, UAC_FREQUENCY_ANY, 6400);

    //Register the camera frame callback function
    usb->uacMicRegisterCb(&onMicFrameCallback, NULL);

    usb->start();

    usb->connectWait(1000);
    delay(5000);

    usb->uacMicMute((void *)0);
    delay(5000);

    usb->uacMicVolume((void *)60);

    usb->uacMicSuspend(NULL);
    delay(5000);

    usb->uacMicResume(NULL);

}

// The loop function runs repeatedly
void loop()
{
    // Delay the task for 100ms
    vTaskDelay(5000);
}
