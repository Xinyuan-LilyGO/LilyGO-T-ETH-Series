/**
 * @file      ADC_Button.ino
 * @author    Lewis He (lewishe@outlook.com)
 * @license   MIT
 * @copyright Copyright (c) 2024  ShenZhen XinYuan Electronic Technology Co., Ltd
 * @date      2024-06-24
 * @note      The example demonstrates how to use ADC as a key function. Please see the example picture for the external circuit.
 */
#include <Arduino.h>

// Select the board model to be used and adjust the Pin according to the actual situation

// #define LILYGO_T_INTERNET_POE
// #define LILYGO_T_ETH_POE_PRO
// #define LILYGO_T_INTER_COM           //Can't run
// #define LILYGO_T_ETH_LITE_ESP32
// #define LILYGO_T_ETH_LITE_ESP32S3
// #define LILYGO_T_ETH_ELITE_ESP32S3

#if   defined(LILYGO_T_INTERNET_POE)
// IO34 35,39,34,36 can only be used for input,ADC function is limited to GPIO33 and above
#define ADC_PIN     36
#elif defined(LILYGO_T_ETH_POE_PRO)
// IO34 35,39,34,36 can only be used for input,ADC function is limited to GPIO33 and above
#define ADC_PIN     36
#elif defined(LILYGO_T_INTER_COM)
//No free pin
#error "No  free pin"
#elif defined(LILYGO_T_ETH_LITE_ESP32)
// IO35,36,37,38,39 can only be used for input and cannot be set as output
#define ADC_PIN     36
#elif defined(LILYGO_T_ETH_LITE_ESP32S3)
// ESP32S3 can freely map unused Pins , ADC function is limited to GPIO33 and below
#define ADC_PIN     7
#elif defined(LILYGO_T_ETH_ELITE_ESP32S3)
// ESP32S3 can freely map unused Pins , ADC function is limited to GPIO33 and below
#define ADC_PIN     7
#endif

#define SAMPLE_TIME      200
#define DEVIATION        0.1

xQueueHandle adc_queue = NULL;

void button_task(void *arg)
{
    while (1) {
        double voltage = 0;
        voltage = analogReadMilliVolts(ADC_PIN) / 1000.0;
        xQueueSend(adc_queue, (double *)&voltage, 0);
        vTaskDelay(pdMS_TO_TICKS(SAMPLE_TIME));
    }
    vTaskDelete(NULL);
}

void setup()
{
    Serial.begin(115200);
    adc_queue = xQueueCreate(1, sizeof(double));
    analogReadResolution(12);
    analogSetAttenuation(ADC_11db);
    xTaskCreatePinnedToCore(&button_task, "btn", 3 * 1024, NULL, 5, NULL, 0);
}


void loop()
{
    double voltage = 0;
    xQueueReceive(adc_queue, &voltage, portMAX_DELAY);
    if (voltage > 2.6) {
    } else if (voltage > 2.2 - DEVIATION  && voltage <= 2.2 + DEVIATION) {
        Serial.println("B5");
    } else if (voltage > 1.65 - DEVIATION  && voltage <= 1.65 + DEVIATION) {
        Serial.println("B4");
    } else if (voltage > 1.11 - DEVIATION  && voltage <= 1.11 + DEVIATION) {
        Serial.println("B3");
    } else if (voltage > 0.76 - DEVIATION  && voltage <= 0.76 + DEVIATION) {
        Serial.println("B2");
    } else if (voltage > 0.43 - DEVIATION  && voltage <= 0.43 + DEVIATION) {
        Serial.println("B1");
    }
    delay(5);
}












