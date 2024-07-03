/**
 * @file      T-ETH-Elite-LTE-Shield.ino
 * @author    Lewis He (lewishe@outlook.com)
 * @license   MIT
 * @copyright Copyright (c) 2024  ShenZhen XinYuan Electronic Technology Co., Ltd
 * @date      2024-06-5
 * @note      This warehouse does not provide modem examples, only how to communicate and initialize.
 *            For how to use modem, you can check the TinyGSM library or check the AT command manual of the relevant modem.
 *
 * Example links for reference:
 *          You can refer to the example link, written for novice users. Please note that it is a reference,
 *          not a direct operation. The relevant GPIO must be set to be consistent with ELite before running.
 *
 *          - A7670X/A7608X/SIM7670G  :  https://github.com/Xinyuan-LilyGO/LilyGO-T-A76XX
 *          - SIM7600X                :  https://github.com/Xinyuan-LilyGO/T-SIM7600X
 *          - SIM7070G/SIM7000X       :  https://github.com/Xinyuan-LilyGO/LilyGO-T-SIM7000G
 *          - SIM7080G                :  https://github.com/Xinyuan-LilyGO/LilyGO-T-SIM7080G
 *          If the message "No GPS data received" appears, please check whether the GPS switch on the back of the T-ETH-Elite-LTE-Shield board has been turned to the ON side.
 */

#include <Arduino.h>
#include "utilities.h"

// The baud rate may not be suitable for all GPS modules. Adjust the baud rate according to your own GPS module.
#define GPS_BAUD        9600
#define GPSSerial       Serial1
#define SerialAT        Serial2


#include <TinyGPS++.h>
TinyGPSPlus gps;

static bool waitResponse(String &data, String rsp, uint32_t timeout);
static bool waitResponse(String rsp, uint32_t timeout);
static void printStr(const char *str, int len);
static void printDateTime(TinyGPSDate &d, TinyGPSTime &t);
static void printInt(unsigned long val, bool valid, int len);
static void smartDelay(unsigned long ms);
static void printFloat(float val, bool valid, int len, int prec);
static bool checkSimCard();

uint32_t checkAutoBaud()
{
    static uint32_t rates[] = {115200, 9600, 57600,  38400, 19200,  74400, 74880,
                               230400, 460800, 2400,  4800,  14400, 28800
                              };
    for (uint8_t i = 0; i < sizeof(rates) / sizeof(rates[0]); i++) {
        uint32_t rate = rates[i];
        Serial.printf("Trying baud rate %u\n", rate);
        SerialAT.updateBaudRate(rate);
        delay(10);
        for (int j = 0; j < 10; j++) {
            SerialAT.print("AT\r\n");
            String input = SerialAT.readString();
            if (input.indexOf("OK") >= 0) {
                Serial.printf("Modem responded at rate:%u\n", rate);
                return rate;
            }
        }
    }
    SerialAT.updateBaudRate(115200);
    return 0;
}

void setup()
{
    Serial.begin(115200);

    SerialAT.begin(115200, SERIAL_8N1, MODEM_RX_PIN, MODEM_TX_PIN);

    GPSSerial.begin(9600, SERIAL_8N1, GPS_RX_PIN, GPS_TX_PIN);

    Serial.println("Start modem .");

    // Turn on modem
    pinMode(MODEM_PWRKEY_PIN, OUTPUT);
    digitalWrite(MODEM_PWRKEY_PIN, LOW);
    delay(100);
    digitalWrite(MODEM_PWRKEY_PIN, HIGH);
    delay(100);
    digitalWrite(MODEM_PWRKEY_PIN, LOW);

    // Delay sometime wait modem power on
    delay(15000);

    if (!checkAutoBaud()) {
        while (1) {
            Serial.println(F(" Failed to connect to the modem! Check the baud and try again."));
            delay(2000);
        }
    }

    bool simCardOk = false;
    int retry = 0;
    while (retry < 5) {
        simCardOk = checkSimCard();
        if (simCardOk) {
            break;
        }
        retry++;
    }

    if (simCardOk) {
        Serial.println("SIM Card detected PASS");
    } else {
        Serial.println("SIM Card detected FAILED");
        Serial.println("SIM Card detected FAILED");
        Serial.println("SIM Card detected FAILED");
    }


    Serial.println("-----------------");
    Serial.println("GPS Start ......");

}

void loop()
{
    if (gps.location.isValid() && gps.time.isValid() && gps.date.isValid()) {
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
    } else {
        Serial.print(".");
    }
    smartDelay(1000);
    if (millis() > 5000 && gps.charsProcessed() < 10) {
        Serial.println(F("No GPS data received: check wiring"));
        Serial.println(F("Please check whether the GPS switch on the back of the T-ETH-Elite-LTE-Shield board has been turned to the ON side."));
        delay(500);
    }
}


static bool checkSimCard()
{
    SerialAT.println("AT");
    if (waitResponse("OK", 1000)) {
        SerialAT.println("AT+CPIN?");
        if (waitResponse("+CPIN: READ", 1000)) {
            Serial.println("[Modem]: SIM Card detected inserted");
            return true;
        }
    }
    return false;
}


static bool waitResponse(String rsp, uint32_t timeout)
{
    String data;
    return waitResponse(data, rsp, timeout);
}

static bool waitResponse(String &data, String rsp, uint32_t timeout)
{
    uint32_t startMillis = millis();
    do {
        while (SerialAT.available() > 0) {
            int8_t ch = SerialAT.read();
            data += static_cast<char>(ch);
            if (rsp.length() && data.endsWith(rsp)) {
                return true;
            }
        }
    } while (millis() - startMillis < 1000);
    return false;
}




// This custom version of delay() ensures that the gps object
// is being "fed".
static void smartDelay(unsigned long ms)
{
    unsigned long start = millis();
    do {
        while (GPSSerial.available()) {
            // Serial.write(GPSSerial.read());
            gps.encode(GPSSerial.read());
        }
    } while (millis() - start < ms);
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