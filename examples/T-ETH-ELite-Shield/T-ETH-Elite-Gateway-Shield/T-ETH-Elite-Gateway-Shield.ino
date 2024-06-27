/**
 * @file      T-ETH-Elite-Gateway-Shield.ino
 * @author    Lewis He (lewishe@outlook.com)
 * @license   MIT
 * @copyright Copyright (c) 2024  ShenZhen XinYuan Electronic Technology Co., Ltd
 * @date      2024-04-18
 * @note      It is only used to test whether SX1302 is connected and whether GPS is working properly.
 *            If the message "No GPS data received" appears, please check whether the GPS switch on the back of the T-ETH-Elite-Gateway-Shield board has been turned to the ON side.
 */
#include <Arduino.h>
#include <SPI.h>
#include "utilities.h"
#include "loragw_reg.h"
#include "loragw_hal.h"
#include "loragw_spi.h"

#include <TinyGPS++.h>
TinyGPSPlus gps;

#define LGW_TOTALREGS                           1044
#define SX1302_REG_COMMON_CTRL0_CLK32_RIF_CTRL  1
#define LGW_REG_SUCCESS                         0
#define LGW_REG_ERROR                           -1
extern const struct lgw_reg_s loregs[LGW_TOTALREGS + 1];

// The baud rate may not be suitable for all GPS modules. Adjust the baud rate according to your own GPS module.
#define GPS_BAUD        9600
#define GPSSerial       Serial1

void test_loragw_reg();


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


void setup()
{
    Serial.begin(115200);

    GPSSerial.begin(GPS_BAUD, SERIAL_8N1, GPS_RX_PIN, GPS_TX_PIN);

    while (lgw_connect("") != LGW_REG_SUCCESS) {
        while (1) {
            Serial.printf("ERROR: failed to connect\n"); delay(1000);
        }
    }
    test_loragw_reg();

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
        Serial.println(F("Please check whether the GPS switch on the back of the T-ETH-Elite-Gateway-Shield board has been turned to the ON side."));
        delay(500);
    }
}


void test_loragw_reg()
{
    int x, i;
    int32_t val;
    bool error_found = false;
    uint8_t rand_values[LGW_TOTALREGS];
    bool reg_ignored[LGW_TOTALREGS]; /* store register to be ignored */
    uint8_t reg_val;
    uint8_t reg_max;

    uint8_t data = 0;

    /* The following registers cannot be tested this way */
    memset(reg_ignored, 0, sizeof reg_ignored);
    reg_ignored[SX1302_REG_COMMON_CTRL0_CLK32_RIF_CTRL] = true; /* all test fails if we set this one to 1 */

    /* Test 1: read all registers and check default value for non-read-only registers */
    Serial.printf("## TEST#1: read all registers and check default value for non-read-only registers\n");
    error_found = false;
    for (i = 0; i < LGW_TOTALREGS; i++) {
        if (loregs[i].rdon == 0) {
            x = lgw_reg_r(i, &val);
            if (x != LGW_REG_SUCCESS) {
                Serial.printf("ERROR: failed to read register at index %d\n", i);
                return;
            }
            if (val != loregs[i].dflt) {
                Serial.printf("ERROR: default value for register at index %d is %d, should be %d\n", i, val, loregs[i].dflt);
                error_found = true;
            }
        }
    }
    Serial.printf("------------------\n");
    Serial.printf(" TEST#1 %s\n", (error_found == false) ? "PASSED" : "FAILED");
    Serial.printf("------------------\n\n");

    /* Test 2: read/write test on all non-read-only, non-pulse, non-w0clr, non-w1clr registers */
    Serial.printf("## TEST#2: read/write test on all non-read-only, non-pulse, non-w0clr, non-w1clr registers\n");
    /* Write all registers with a random value */
    error_found = false;
    for (i = 0; i < LGW_TOTALREGS; i++) {
        if ((loregs[i].rdon == 0) && (reg_ignored[i] == false)) {
            /* Peek a random value different form the default reg value */
            reg_max = pow(2, loregs[i].leng) - 1;
            if (loregs[i].leng == 1) {
                reg_val = !loregs[i].dflt;
            } else {
                /* ensure random value is not the default one */
                do {
                    if (loregs[i].sign == 1) {
                        reg_val = rand() % (reg_max / 2);
                    } else {
                        reg_val = rand() % reg_max;
                    }
                } while (reg_val == loregs[i].dflt);
            }
            /* Write selected value */
            x = lgw_reg_w(i, reg_val);
            if (x != LGW_REG_SUCCESS) {
                Serial.printf("ERROR: failed to read register at index %d\n", i);
                return ;
            }
            /* store value for later check */
            rand_values[i] = reg_val;
        }
    }
    /* Read all registers and check if we got proper random value back */
    for (i = 0; i < LGW_TOTALREGS; i++) {
        if ((loregs[i].rdon == 0) && (loregs[i].chck == 1) && (reg_ignored[i] == false)) {
            x = lgw_reg_r(i, &val);
            if (x != LGW_REG_SUCCESS) {
                Serial.printf("ERROR: failed to read register at index %d\n", i);
                return ;
            }
            /* check value */
            if (val != rand_values[i]) {
                Serial.printf("ERROR: value read from register at index %d differs from the written value (w:%u r:%d)\n", i, rand_values[i], val);
                error_found = true;
            } else {
                //Serial.printf("INFO: MATCH reg %d (%u, %u)\n", i, rand_values[i], (uint8_t)val);
            }
        }
    }
    Serial.printf("------------------\n");
    Serial.printf(" TEST#2 %s\n", (error_found == false) ? "PASSED" : "FAILED");
    Serial.printf("------------------\n\n");
    delay(2000);
}