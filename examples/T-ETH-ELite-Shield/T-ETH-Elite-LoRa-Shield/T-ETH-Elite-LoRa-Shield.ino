/**
 * @file      LoRaShield.ino
 * @author    Lewis He (lewishe@outlook.com)
 * @license   MIT
 * @copyright Copyright (c) 2024  ShenZhen XinYuan Electronic Technology Co., Ltd
 * @date      2024-06-22
 * @note      The example demonstrates the functions of all peripherals on the T-ETH-Elite-LoRa-Shield
  *           If the message "No GPS data received" appears, please check whether the GPS switch on the back of the T-ETH-Elite-LoRa-Shield board has been turned to the ON side.
 */
#include "config.h"
#include <TinyGPS++.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>
#if ESP_ARDUINO_VERSION < ESP_ARDUINO_VERSION_VAL(3,0,0)
#include <ETHClass2.h>       //Is to use the modified ETHClass
#define ETH  ETH2
#else
#include <ETH.h>
#endif

enum adc_buttonF {
    ADC_BUTTON_NONE,
    ADC_BUTTON_1,
    ADC_BUTTON_2,
    ADC_BUTTON_3,
    ADC_BUTTON_4,
    ADC_BUTTON_5,
};

#define GPSSerial       Serial1
#define GPS_BAUD        9600
#define SAMPLE_TIME      200
#define DEVIATION        0.1
#define SEALEVELPRESSURE_HPA (1013.25)

float current_freq = CONFIG_RADIO_FREQ;

TinyGPSPlus gps;
Adafruit_BME280 bme;

static xSemaphoreHandle btnSemap = NULL;
static xQueueHandle adc_queue = NULL;
// save transmission state between loops
static int transmissionState = RADIOLIB_ERR_NONE;
// flag to indicate that a packet was sent
static volatile bool transmittedFlag = false;
static uint32_t counter = 0;
static String payload = "None";

static uint32_t last_btn_num = ADC_BUTTON_1;
static bool last_recv_status = false;
static int prev_state;
static int curr_state = HIGH;

void setFlag(void)
{
    // we sent a packet, set the flag
    transmittedFlag = true;
}



void setup()
{
    Serial.begin(115200);

    // Register eth envet
    WiFi.onEvent(WiFiEvent);

    // Onboard LED is a multiplexed function
    // SX1276-TCXO: LED IO38 is multiplexed as TCXO enable function
    // SX1280PA: LED IO38 is multiplexed as LoRa (TX) antenna control function enable function
    // When using other model modules, it is only LED function
#ifdef TCXO_ENABLE_PIN
    pinMode(TCXO_ENABLE_PIN, OUTPUT);
    digitalWrite(TCXO_ENABLE_PIN, HIGH);
#else
    pinMode(LED_PIN, OUTPUT);
    digitalWrite(LED_PIN, HIGH);
#endif

    // Initialize SPI bus
    SPI.begin(SPI_SCLK_PIN, SPI_MISO_PIN, SPI_MOSI_PIN);

    // Radio and SD card share the bus. During initialization, their respective CS Pin needs to be set to HIGH.
    pinMode(RADIO_CS_PIN, OUTPUT);
    digitalWrite(RADIO_CS_PIN, HIGH);
    // Radio and SD card share the bus. During initialization, their respective CS Pin needs to be set to HIGH.
    pinMode(SD_CS_PIN, OUTPUT);
    digitalWrite(SD_CS_PIN, HIGH);

    // Initialize sdcard
    if (!SD.begin(SD_CS_PIN)) {
        Serial.println("Warning: Failed to init SD card");
    } else {
        Serial.println("SD card init succeeded, size : ");
        Serial.print(SD.cardSize() / (1024 * 1024));
        Serial.println("MBytes");
    }

    // Initialize gps serial
    GPSSerial.begin(GPS_BAUD, SERIAL_8N1, GPS_RX_PIN, GPS_TX_PIN);

    // Initialize wire bus
    Wire.begin(I2C_SDA_PIN, I2C_SCL_PIN);

    // Initialize ethernet
    if (!ETH.begin(ETH_PHY_W5500, 1, ETH_CS_PIN, ETH_INT_PIN, ETH_RST_PIN,
                   SPI3_HOST,
                   ETH_SCLK_PIN, ETH_MISO_PIN, ETH_MOSI_PIN)) {
        Serial.println("ETH start Failed!");
    }

    // Initialize adc button
    beginButton();

    // Initialize ssd1306 oled
    beginDisplay();

    // Initialize bme280 sensor
    bool isSensorOnline = beginSensor();

    // Initialize LoRa
    bool isRadioOnline =  beginRadio();


    printResult(isRadioOnline, isSensorOnline);

    // init boot button
    pinMode(0, INPUT_PULLUP);
}



// Only used for LR1121 frequency switching function
void switch_freq()
{
#ifdef CONFIG_RADIO_FREQ1
    if (current_freq == CONFIG_RADIO_FREQ) {
        current_freq = CONFIG_RADIO_FREQ1;
        if (radio.setFrequency(current_freq, false) == RADIOLIB_ERR_INVALID_FREQUENCY) {
            Serial.printf("Selected frequency %f is invalid for this module!\n", current_freq);
        }
        if (radio.setOutputPower(CONFIG_RADIO_OUTPUT_POWER1) == RADIOLIB_ERR_INVALID_OUTPUT_POWER) {
            Serial.printf("Selected output power %d is invalid for this module!\n", CONFIG_RADIO_OUTPUT_POWER1);
        }

    } else {
        current_freq = CONFIG_RADIO_FREQ;
        if (radio.setFrequency(current_freq, false) == RADIOLIB_ERR_INVALID_FREQUENCY) {
            Serial.printf("Selected frequency %f is invalid for this module!\n", current_freq);
        }
        if (radio.setOutputPower(CONFIG_RADIO_OUTPUT_POWER) == RADIOLIB_ERR_INVALID_OUTPUT_POWER) {
            Serial.printf("Selected output power %d is invalid for this module!\n", CONFIG_RADIO_OUTPUT_POWER);
        }
    }
#endif
}



void loop()
{

    // BOOT Pin Swtcih  Freq1
#ifdef CONFIG_RADIO_FREQ1
    prev_state = curr_state;
    curr_state = digitalRead(0);
    if (prev_state == HIGH && curr_state == LOW) {
        switch_freq();
        last_recv_status = false;
    }
#endif

    xSemaphoreTake(btnSemap, portMAX_DELAY);
    uint32_t btn = last_btn_num;
    xSemaphoreGive(btnSemap);
    switch (btn) {
    case ADC_BUTTON_1:  // GPS
        dispGPS();
        last_recv_status = false;
        break;
    case ADC_BUTTON_2:  // Radio Tx
        dispRadioTx();
        last_recv_status = false;
        break;
    case ADC_BUTTON_3:  // Radio Rx
        dispRadioRx();
        break;
    case ADC_BUTTON_4:  // Sensor
        dispSensor();
        last_recv_status = false;
        break;
    case ADC_BUTTON_5:  // NetWork
        last_recv_status = false;
        dispNetwork();
        break;
    default:
        break;
    }
    smartDelay(150);
}

void smartDelay(uint32_t ms)
{
    static uint32_t interval = 0;
    uint32_t start = millis();
    do {
        while (GPSSerial.available()) {
            // Serial.write(GPSSerial.read());
            gps.encode(GPSSerial.read());
        }
    } while (millis() - start < ms);

    // Check GPS data
    if (millis() > 15000 && gps.charsProcessed() < 30) {
        if (millis() > interval) {
            Serial.println(F("No GPS data received: check wiring"));
            Serial.println(F("Please check whether the GPS switch on the back of the T-ETH-Elite-LoRa-Shield board has been turned to the ON side."));
            interval = millis() + 2000;
        }
    }
}

void dispNetwork()
{
    if (u8g2) {
        u8g2->clearBuffer();
        u8g2->drawRFrame(0, 0, 128, 64, 5);
        u8g2->setFont(u8g2_font_crox1h_tr);
        u8g2->setCursor(10, 15);
        u8g2->print("IP:"); u8g2->print(ETH.localIP().toString());
        u8g2->setCursor(10, 30);
        u8g2->print("MAC:"); u8g2->print(ETH.macAddress());
        u8g2->sendBuffer();
    } else {
        Serial.println("-------------B5-----------------------");
        Serial.print("IP:"); Serial.println(ETH.localIP().toString());
        Serial.print("MAC:"); Serial.println(ETH.macAddress());
        delay(1000);
    }
}

void dispGPS()
{
    if (u8g2) {
        u8g2->clearBuffer();
        u8g2->drawRFrame(0, 0, 128, 64, 5);
        u8g2->setFont(u8g2_font_crox1h_tr);
        u8g2->setCursor(10, 15);
        u8g2->print("lat:"); u8g2->print(gps.location.isValid() ? gps.location.lat() : 0, 5);
        u8g2->setCursor(10, 30);
        u8g2->print("lng:"); u8g2->print(gps.location.isValid() ? gps.location.lng() : 0, 5);
        u8g2->setCursor(10, 45);
        u8g2->print("Time:");
        if (gps.date.isValid() && gps.time.isValid()) {
            u8g2->print(gps.time.hour());
            u8g2->print(":");
            u8g2->print(gps.time.minute());
            u8g2->print(":");
            u8g2->print(gps.time.second());
        } else {
            u8g2->print("00:00:00");
        }
        u8g2->setCursor(10, 60);
        u8g2->print("Rx:");
        u8g2->print(gps.charsProcessed());
        u8g2->sendBuffer();
    } else {
        printInt(gps.satellites.value(), gps.satellites.isValid(), 5);
        printFloat(gps.hdop.hdop(), gps.hdop.isValid(), 6, 1);
        printFloat(gps.location.lat(), gps.location.isValid(), 11, 6);
        printFloat(gps.location.lng(), gps.location.isValid(), 12, 6);
        printInt(gps.location.age(), gps.location.isValid(), 5);
        printDateTime(gps.date, gps.time);
        printFloat(gps.altitude.meters(), gps.altitude.isValid(), 7, 2);
        printFloat(gps.course.deg(), gps.course.isValid(), 7, 2);
        printFloat(gps.speed.kmph(), gps.speed.isValid(), 6, 2);
        printStr(gps.course.isValid() ? TinyGPSPlus::cardinal(gps.course.deg()) : "*** ", 6);
        printInt(gps.charsProcessed(), true, 6);
        printInt(gps.sentencesWithFix(), true, 10);
        printInt(gps.failedChecksum(), true, 9);
        Serial.println();
        delay(1000);
    }
}

void dispRadioTx()
{
    static uint32_t last_send_millis = 0;


    // check if the previous transmission finished
    if (transmittedFlag) {

        payload = "#" + String(counter++);

        // reset flag
        transmittedFlag = false;

        Serial.print(F("Radio Sending another packet ... "));

        if (transmissionState == RADIOLIB_ERR_NONE) {
            // packet was successfully sent
            Serial.println(F("transmission finished!"));
            // NOTE: when using interrupt-driven transmit method,
            //       it is not possible to automatically measure
            //       transmission data rate using getDataRate()
        } else {
            Serial.print(F("failed, code "));
            Serial.println(transmissionState);
        }

        if (u8g2) {
            u8g2->clearBuffer();
            u8g2->drawRFrame(0, 0, 128, 64, 5);

            u8g2->setFont(u8g2_font_crox1h_tr);
            u8g2->setCursor(15, 15);
            u8g2->print("TX:");
            u8g2->setCursor(15, 30);
            u8g2->print("STATE:");
            u8g2->setCursor(15, 45);
            u8g2->print("FREQ:");

            // u8g2->setFont(u8g2_font_crox1h_tr);
            u8g2->setCursor( U8G2_HOR_ALIGN_RIGHT(payload.c_str()) - 14, 15 );
            u8g2->print(payload);
            String state = transmissionState == RADIOLIB_ERR_NONE ? "NONE" : String(transmissionState);
            u8g2->setCursor( U8G2_HOR_ALIGN_RIGHT(state.c_str()) -  14, 30 );
            u8g2->print(state);
            String freq = String(current_freq) + "MHz";
            u8g2->setCursor( U8G2_HOR_ALIGN_RIGHT(freq.c_str()) -  14, 45 );
            u8g2->print(freq);
            u8g2->sendBuffer();
        } else {
            Serial.println("-------------B2-----------------------");
            Serial.println("Radio Tx ...");
            Serial.print("FREQ:"); Serial.print(current_freq); Serial.println("MHz");
        }

    }

    if (millis() > last_send_millis) {
        // send another one
        //

        // you can transmit C-string or Arduino string up to
        // 256 characters long
        transmissionState = radio.startTransmit(payload);
        // you can also transmit byte array up to 256 bytes long
        /*
          byte byteArr[] = {0x01, 0x23, 0x45, 0x67,
                            0x89, 0xAB, 0xCD, 0xEF};
          int state = radio.startTransmit(byteArr, 8);
        */
        last_send_millis = millis() + 1000;
    }
}

void drawRadioRx(String payload, String snr, String rssi)
{
    if (u8g2) {
        u8g2->clearBuffer();
        u8g2->drawRFrame(0, 0, 128, 64, 5);
        u8g2->setFont(u8g2_font_crox1h_tr);
        u8g2->setCursor(15, 15);
        u8g2->print("RX:");
        u8g2->setCursor(15, 30);
        u8g2->print("SNR:");
        u8g2->setCursor(15, 45);
        u8g2->print("RSSI:");
        u8g2->setCursor(15, 58);
        u8g2->print("FREQ:");

        // u8g2->setFont(u8g2_font_crox1h_tr);
        u8g2->setCursor( U8G2_HOR_ALIGN_RIGHT(payload.c_str()) - 21, 15 );
        u8g2->print(payload);
        u8g2->setCursor( U8G2_HOR_ALIGN_RIGHT(snr.c_str()) - 21, 30 );
        u8g2->print(snr);
        u8g2->setCursor( U8G2_HOR_ALIGN_RIGHT(rssi.c_str()) - 21, 45 );
        u8g2->print(rssi);
        u8g2->setCursor( U8G2_HOR_ALIGN_RIGHT(String(current_freq).c_str()) - 21, 58 );
        u8g2->print(String(current_freq));
        u8g2->sendBuffer();
    } else {

        Serial.println("-------------B3-----------------------");
        Serial.println("Radio Rx ...");
        Serial.print("FREQ:"); Serial.print(current_freq); Serial.println("MHz");
    }
}

void dispRadioRx()
{

    if (!last_recv_status) {
        transmissionState = radio.startReceive();
        last_recv_status = true;
        drawRadioRx("NONE", "0dB", "0dBm");
    }

    // check if the flag is set
    if (transmittedFlag) {

        // reset flag
        transmittedFlag = false;

        // you can read received data as an Arduino String
        int state = radio.readData(payload);

        // you can also read received data as byte array
        /*
          byte byteArr[8];
          int state = radio.readData(byteArr, 8);
        */

        if (state == RADIOLIB_ERR_NONE) {

            String rssi = String(radio.getRSSI()) + "dBm";
            String snr = String(radio.getSNR()) + "dB";

            drawRadioRx(payload, snr, rssi);

            // packet was successfully received
            Serial.println(F("Radio Received packet!"));

            // print data of the packet
            Serial.print(F("Radio Data:\t\t"));
            Serial.println(payload);

            // print RSSI (Received Signal Strength Indicator)
            Serial.print(F("Radio RSSI:\t\t"));
            Serial.println(rssi);

            // print SNR (Signal-to-Noise Ratio)
            Serial.print(F("Radio SNR:\t\t"));
            Serial.println(snr);

        } else if (state == RADIOLIB_ERR_CRC_MISMATCH) {
            // packet was received, but is malformed
            Serial.println(F("CRC error!"));
        } else {
            // some other error occurred
            Serial.print(F("failed, code "));
            Serial.println(state);
        }

        // put module back to listen mode
        radio.startReceive();

    }
}

void dispSensor()
{
    if (u8g2) {
        u8g2->clearBuffer();

        u8g2->drawRFrame(0, 0, 128, 64, 5);
        u8g2->setFont(u8g2_font_crox1h_tr);

        u8g2->setCursor(10, 15);

        u8g2->print("Temperature:");
        u8g2->print(bme.readTemperature());
        u8g2->print(" *C");

        u8g2->setCursor(10, 30);
        u8g2->print("Pressure:");
        u8g2->println(bme.readPressure() / 100.0F);
        u8g2->print(" hPa");

        u8g2->setCursor(10, 45);
        u8g2->print("Altitude:");
        u8g2->println(bme.readAltitude(SEALEVELPRESSURE_HPA));
        u8g2->print(" m");

        u8g2->setCursor(10, 58);
        u8g2->print("Humidity:");
        u8g2->println(bme.readHumidity());
        u8g2->print(" %");

        u8g2->sendBuffer();
    } else {

        Serial.println("-------------B4-----------------------");

        Serial.print("Temperature = ");
        Serial.print(bme.readTemperature());
        Serial.println(" *C");

        Serial.print("Pressure = ");

        Serial.print(bme.readPressure() / 100.0F);
        Serial.println(" hPa");

        Serial.print("Approx. Altitude = ");
        Serial.print(bme.readAltitude(SEALEVELPRESSURE_HPA));
        Serial.println(" m");

        Serial.print("Humidity = ");
        Serial.print(bme.readHumidity());
        Serial.println(" %");

        Serial.println();

        delay(500);
    }
}

void button_task(void *arg)
{
    uint32_t btn_number = ADC_BUTTON_NONE;
    while (1) {
        double voltage = analogReadMilliVolts(7) / 1000.0;
        if (voltage > 2.2 - DEVIATION  && voltage <= 2.2 + DEVIATION) {
            btn_number = ADC_BUTTON_5;
        } else if (voltage > 1.65 - DEVIATION  && voltage <= 1.65 + DEVIATION) {
            btn_number = ADC_BUTTON_4;
        } else if (voltage > 1.11 - DEVIATION  && voltage <= 1.11 + DEVIATION) {
            btn_number = ADC_BUTTON_3;
        } else if (voltage > 0.76 - DEVIATION  && voltage <= 0.76 + DEVIATION) {
            btn_number = ADC_BUTTON_2;
        } else if (voltage > 0.43 - DEVIATION  && voltage <= 0.43 + DEVIATION) {
            btn_number = ADC_BUTTON_1;
        }
        xSemaphoreTake(btnSemap, portMAX_DELAY);
        if (btn_number != last_btn_num) {
            last_btn_num = btn_number;
        }
        xSemaphoreGive(btnSemap);
        vTaskDelay(pdMS_TO_TICKS(SAMPLE_TIME));
    }
    vTaskDelete(NULL);
}

void beginButton()
{
    btnSemap = xSemaphoreCreateBinary();
    xSemaphoreGive(btnSemap);
    adc_queue = xQueueCreate(1, sizeof(uint32_t));
    analogReadResolution(12);
    analogSetAttenuation(ADC_11db);
    xTaskCreatePinnedToCore(&button_task, "btn", 3 * 1024, NULL, 5, NULL, 0);
}

bool beginSensor()
{
    // default settings
    bool status = bme.begin();
    // You can also pass in a Wire library object like &Wire2
    // status = bme.begin(0x76, &Wire2)
    if (!status) {
        Serial.println("Could not find a valid BME280 sensor, check wiring, address, sensor ID!");
        Serial.print("SensorID was: 0x"); Serial.println(bme.sensorID(), 16);
        Serial.print("        ID of 0xFF probably means a bad address, a BMP 180 or BMP 085\n");
        Serial.print("   ID of 0x56-0x58 represents a BMP 280,\n");
        Serial.print("        ID of 0x60 represents a BME 280.\n");
        Serial.print("        ID of 0x61 represents a BME 680.\n");
        if (u8g2) {
            u8g2->setFont(u8g2_font_ncenB08_tr);
            u8g2->clearBuffer();
            u8g2->setCursor(0, 16);
            u8g2->print("BME280 Could not find");
            u8g2->sendBuffer();
        }
        return false;
    }
    return true;
}

bool beginRadio()
{
    // initialize radio with default settings
    int state = radio.begin();

    Serial.print(F("Radio Initializing ... "));
    if (state == RADIOLIB_ERR_NONE) {
        Serial.println(F("success!"));
    } else {
        Serial.print(F("failed, code "));
        Serial.println(state);
        return false;
    }

    // set the function that will be called
    // when packet transmission is finished
    radio.setPacketSentAction(setFlag);

    /*
    *   Sets carrier frequency.
    *   SX1278/SX1276 : Allowed values range from 137.0 MHz to 525.0 MHz.
    *   SX1268/SX1262 : Allowed values are in range from 150.0 to 960.0 MHz.
    *   SX1280        : Allowed values are in range from 2400.0 to 2500.0 MHz.
    *   LR1121        : Allowed values are in range from 150.0 to 960.0 MHz, 1900 - 2200 MHz and 2400 - 2500 MHz. Will also perform calibrations.
    * * * */

    if (radio.setFrequency(current_freq) == RADIOLIB_ERR_INVALID_FREQUENCY) {
        Serial.println(F("Selected frequency is invalid for this module!"));
        return false;
    }

    /*
    *   Sets LoRa link bandwidth.
    *   SX1278/SX1276 : Allowed values are 10.4, 15.6, 20.8, 31.25, 41.7, 62.5, 125, 250 and 500 kHz. Only available in %LoRa mode.
    *   SX1268/SX1262 : Allowed values are 7.8, 10.4, 15.6, 20.8, 31.25, 41.7, 62.5, 125.0, 250.0 and 500.0 kHz.
    *   SX1280        : Allowed values are 203.125, 406.25, 812.5 and 1625.0 kHz.
    *   LR1121        : Allowed values are 62.5, 125.0, 250.0 and 500.0 kHz.
    * * * */
    if (radio.setBandwidth(CONFIG_RADIO_BW) == RADIOLIB_ERR_INVALID_BANDWIDTH) {
        Serial.println(F("Selected bandwidth is invalid for this module!"));
        return false;
    }


    /*
    * Sets LoRa link spreading factor.
    * SX1278/SX1276 :  Allowed values range from 6 to 12. Only available in LoRa mode.
    * SX1262        :  Allowed values range from 5 to 12.
    * SX1280        :  Allowed values range from 5 to 12.
    * LR1121        :  Allowed values range from 5 to 12.
    * * * */
    if (radio.setSpreadingFactor(12) == RADIOLIB_ERR_INVALID_SPREADING_FACTOR) {
        Serial.println(F("Selected spreading factor is invalid for this module!"));
        return false;
    }

    /*
    * Sets LoRa coding rate denominator.
    * SX1278/SX1276/SX1268/SX1262 : Allowed values range from 5 to 8. Only available in LoRa mode.
    * SX1280        :  Allowed values range from 5 to 8.
    * LR1121        :  Allowed values range from 5 to 8.
    * * * */
    if (radio.setCodingRate(6) == RADIOLIB_ERR_INVALID_CODING_RATE) {
        Serial.println(F("Selected coding rate is invalid for this module!"));
        return false;
    }

    /*
    * Sets LoRa sync word.
    * SX1278/SX1276/SX1268/SX1262/SX1280 : Sets LoRa sync word. Only available in LoRa mode.
    * * */
    if (radio.setSyncWord(0xAB) != RADIOLIB_ERR_NONE) {
        Serial.println(F("Unable to set sync word!"));
        return false;
    }

    /*
    * Sets transmission output power.
    * SX1278/SX1276 :  Allowed values range from -3 to 15 dBm (RFO pin) or +2 to +17 dBm (PA_BOOST pin). High power +20 dBm operation is also supported, on the PA_BOOST pin. Defaults to PA_BOOST.
    * SX1262        :  Allowed values are in range from -9 to 22 dBm. This method is virtual to allow override from the SX1261 class.
    * SX1268        :  Allowed values are in range from -9 to 22 dBm.
    * SX1280        :  Allowed values are in range from -18 to 13 dBm. PA Version range : -18 ~ 3dBm
    * LR1121        :  Allowed values are in range from -9 to 22 dBm (high-power PA) or -17 to 14 dBm (low-power PA)
    * * * */
    if (radio.setOutputPower(CONFIG_RADIO_OUTPUT_POWER) == RADIOLIB_ERR_INVALID_OUTPUT_POWER) {
        Serial.println(F("Selected output power is invalid for this module!"));
        return false;
    }

#if !defined(USING_SX1280) && !defined(USING_SX1280PA) && !defined(USING_LR1121)
    /*
    * Sets current limit for over current protection at transmitter amplifier.
    * SX1278/SX1276 : Allowed values range from 45 to 120 mA in 5 mA steps and 120 to 240 mA in 10 mA steps.
    * SX1262/SX1268 : Allowed values range from 45 to 120 mA in 2.5 mA steps and 120 to 240 mA in 10 mA steps.
    * NOTE: set value to 0 to disable overcurrent protection
    * * * */
    if (radio.setCurrentLimit(140) == RADIOLIB_ERR_INVALID_CURRENT_LIMIT) {
        Serial.println(F("Selected current limit is invalid for this module!"));
        return false;
    }
#endif

    /*
    * Sets preamble length for LoRa or FSK modem.
    * SX1278/SX1276 : Allowed values range from 6 to 65535 in %LoRa mode or 0 to 65535 in FSK mode.
    * SX1262/SX1268 : Allowed values range from 1 to 65535.
    * SX1280        : Allowed values range from 1 to 65535. preamble length is multiple of 4
    * LR1121        : Allowed values range from 1 to 65535.
    * * */
    if (radio.setPreambleLength(16) == RADIOLIB_ERR_INVALID_PREAMBLE_LENGTH) {
        Serial.println(F("Selected preamble length is invalid for this module!"));
        return false;
    }

    // Enables or disables CRC check of received packets.
    if (radio.setCRC(false) == RADIOLIB_ERR_INVALID_CRC_CONFIGURATION) {
        Serial.println(F("Selected CRC is invalid for this module!"));
        return false;
    }

// RADIOLIB_LR11X0_RFSW_DIO6_ENABLED
#ifdef USING_DIO2_AS_RF_SWITCH
#ifdef USING_SX1262
    // Some SX126x modules use DIO2 as RF switch. To enable
    // this feature, the following method can be used.
    // NOTE: As long as DIO2 is configured to control RF switch,
    //       it can't be used as interrupt pin!
    if (radio.setDio2AsRfSwitch() != RADIOLIB_ERR_NONE) {
        Serial.println(F("Failed to set DIO2 as RF switch!"));
        return false;
    }
#endif //USING_SX1262
#endif //USING_DIO2_AS_RF_SWITCH


#if  defined(USING_LR1121)
    // LR1121
    // set RF switch configuration for Wio WM1110
    // Wio WM1110 uses DIO5 and DIO6 for RF switching
    static const uint32_t rfswitch_dio_pins[] = {
        RADIOLIB_LR11X0_DIO5, RADIOLIB_LR11X0_DIO6,
        RADIOLIB_NC, RADIOLIB_NC, RADIOLIB_NC
    };

    static const Module::RfSwitchMode_t rfswitch_table[] = {
        // mode                  DIO5  DIO6
        { LR11x0::MODE_STBY,   { LOW,  LOW  } },
        { LR11x0::MODE_RX,     { HIGH, LOW  } },
        { LR11x0::MODE_TX,     { LOW,  HIGH } },
        { LR11x0::MODE_TX_HP,  { LOW,  HIGH } },
        { LR11x0::MODE_TX_HF,  { LOW,  LOW  } },
        { LR11x0::MODE_GNSS,   { LOW,  LOW  } },
        { LR11x0::MODE_WIFI,   { LOW,  LOW  } },
        END_OF_MODE_TABLE,
    };
    radio.setRfSwitchTable(rfswitch_dio_pins, rfswitch_table);

    // LR1121 TCXO Voltage 2.85~3.15V
    radio.setTCXO(3.0);
#endif

#ifdef RADIO_RX_PIN
    // SX1280 PA Version
    radio.setRfSwitchPins(RADIO_RX_PIN, RADIO_TX_PIN);
#endif

    // start transmitting the first packet
    Serial.print(F("Radio Sending first packet ... "));

    // you can transmit C-string or Arduino string up to
    // 256 characters long
    transmissionState = radio.startTransmit(String(counter).c_str());

    // you can also transmit byte array up to 256 bytes long
    /*
      byte byteArr[] = {0x01, 0x23, 0x45, 0x67,
                        0x89, 0xAB, 0xCD, 0xEF};
      state = radio.startTransmit(byteArr, 8);
    */

    return true;
}


bool beginDisplay()
{
    Wire.beginTransmission(DISPLAY_ADDR);
    if (Wire.endTransmission() == 0) {
        Serial.printf("Find Display model at 0x%X address\n", DISPLAY_ADDR);
        u8g2 = new DISPLAY_MODEL(U8G2_R0, U8X8_PIN_NONE);
        u8g2->begin();
        u8g2->clearBuffer();
        u8g2->setFont(u8g2_font_inb19_mr);
        u8g2->drawStr(0, 30, "LilyGo");
        u8g2->drawHLine(2, 35, 47);
        u8g2->drawHLine(3, 36, 47);
        u8g2->drawVLine(45, 32, 12);
        u8g2->drawVLine(46, 33, 12);
        u8g2->setFont(u8g2_font_inb19_mf);
        u8g2->drawStr(58, 60, "LoRa");
        u8g2->sendBuffer();
        u8g2->setFont(u8g2_font_fur11_tf);
        delay(3000);
        return true;
    }

    Serial.printf("Warning: Failed to find Display at 0x%0X address\n", DISPLAY_ADDR);
    return false;
}



void printResult(bool radio, bool sensor)
{
    Serial.print("Radio        : ");
    Serial.println((radio) ? "+" : "-");

    Serial.print("PSRAM        : ");
    Serial.println((psramFound()) ? "+" : "-");

    Serial.print("Display      : ");
    Serial.println(( u8g2) ? "+" : "-");

    Serial.print("Sd Card      : ");
    Serial.println((SD.cardSize() != 0) ? "+" : "-");

    Serial.print("Sensor        : ");
    Serial.println((sensor) ? "+" : "-");


    if (u8g2) {
        u8g2->clearBuffer();
        u8g2->setFont(u8g2_font_NokiaLargeBold_tf );
        uint16_t str_w =  u8g2->getStrWidth("T-ETH");
        u8g2->drawStr((u8g2->getWidth() - str_w) / 2, 16, "T-ETH");
        u8g2->drawHLine(5, 21, u8g2->getWidth() - 5);
        u8g2->drawStr( 0, 38, "Disp:");     u8g2->drawStr( 45, 38, ( u8g2) ? "+" : "-");
        u8g2->drawStr( 0, 54, "SD :");      u8g2->drawStr( 45, 54, (SD.cardSize() != 0) ? "+" : "-");
        u8g2->drawStr( 62, 38, "Radio:");    u8g2->drawStr( 120, 38, ( radio ) ? "+" : "-");
        u8g2->drawStr( 62, 54, "Sensor:");    u8g2->drawStr( 120, 54, ( sensor ) ? "+" : "-");
        u8g2->sendBuffer();

    }
}


static void printFloat(float val, bool valid, int len, int prec)
{
    if (!valid) {
        while (len-- > 1)
            Serial.print('*');
        Serial.print(' ');
    } else {
        Serial.print(val, prec);
        int vi = abs((int)val);
        int flen = prec + (val < 0.0 ? 2 : 1); // . and -
        flen += vi >= 1000 ? 4 : vi >= 100 ? 3 : vi >= 10 ? 2 : 1;
        for (int i = flen; i < len; ++i)
            Serial.print(' ');
    }
    smartDelay(0);
}

static void printInt(unsigned long val, bool valid, int len)
{
    char sz[32] = "*****************";
    if (valid)
        sprintf(sz, "%ld", val);
    sz[len] = 0;
    for (int i = strlen(sz); i < len; ++i)
        sz[i] = ' ';
    if (len > 0)
        sz[len - 1] = ' ';
    Serial.print(sz);
    smartDelay(0);
}

static void printDateTime(TinyGPSDate &d, TinyGPSTime &t)
{
    if (!d.isValid()) {
        Serial.print(F("********** "));
    } else {
        char sz[32];
        sprintf(sz, "%02d/%02d/%02d ", d.month(), d.day(), d.year());
        Serial.print(sz);
    }

    if (!t.isValid()) {
        Serial.print(F("******** "));
    } else {
        char sz[32];
        sprintf(sz, "%02d:%02d:%02d ", t.hour(), t.minute(), t.second());
        Serial.print(sz);
    }

    printInt(d.age(), d.isValid(), 5);
    smartDelay(0);
}

static void printStr(const char *str, int len)
{
    int slen = strlen(str);
    for (int i = 0; i < len; ++i)
        Serial.print(i < slen ? str[i] : ' ');
    smartDelay(0);
}


void WiFiEvent(arduino_event_id_t event)
{
    switch (event) {
    case ARDUINO_EVENT_ETH_START:
        Serial.println("ETH Started");
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
        break;
    case ARDUINO_EVENT_ETH_DISCONNECTED:
        Serial.println("ETH Disconnected");
        break;
    case ARDUINO_EVENT_ETH_STOP:
        Serial.println("ETH Stopped");
        break;
    default:
        break;
    }
}
