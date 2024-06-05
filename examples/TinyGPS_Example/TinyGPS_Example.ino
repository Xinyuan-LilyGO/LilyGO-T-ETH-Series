/**
 * @file      TinyGPS_FullExample.ino
 * @author    Lewis He (lewishe@outlook.com)
 * @license   MIT
 * @copyright Copyright (c) 2024  ShenZhen XinYuan Electronic Technology Co., Ltd
 * @date      2024-06-05
 * @note      
 */

#include <TinyGPS++.h>

/*
* The following GPIOs can be freely set. Except for the output GPIO limit of ESP32, ESP32S3 can freely map each unused GPIO
* T-ETH-ELITE default GPS GPIO is defined in "utilities.h" ， It needs to be used with an expansion board. The standalone T-ETH-ELITE can be used without GPS.
* */

#if   defined(LILYGO_T_INTERNET_POE)
// IO34 35,39,34,36 can only be used for input and cannot be set as output
#define GPS_RX_PIN     14
#define GPS_TX_PIN     15
#elif defined(LILYGO_T_ETH_POE_PRO)
// IO34 35,39,34,36 can only be used for input and cannot be set as output
#define GPS_RX_PIN     13
#define GPS_TX_PIN     14
#elif defined(LILYGO_T_INTER_COM)
#error "There is no free GPIO, expansion is not possible, and this example cannot be used"
#elif defined(LILYGO_T_ETH_LITE_ESP32)
// IO35,36,37,38,39 can only be used for input and cannot be set as output
#define GPS_RX_PIN     13
#define GPS_TX_PIN     14
#elif defined(LILYGO_T_ETH_LITE_ESP32S3)
// ESP32S3 can freely map unused Pins
#define GPS_RX_PIN     1
#define GPS_TX_PIN     2
#endif

// The baud rate may not be suitable for all GPS modules. Adjust the baud rate according to your own GPS module.
#define GPS_BAUD        9600

#define GPSSerial       Serial1

TinyGPSPlus gps;

void setup()
{
    Serial.begin(115200);

    GPSSerial.begin(GPS_BAUD, SERIAL_8N1, GPS_RX_PIN, GPS_TX_PIN);

    Serial.println(F("FullExample.ino"));
    Serial.println(F("An extensive example of many interesting TinyGPS++ features"));
    Serial.print(F("Testing TinyGPS++ library v. ")); Serial.println(TinyGPSPlus::libraryVersion());
    Serial.println(F("by Mikal Hart"));
    Serial.println();
    Serial.println(F("Sats HDOP  Latitude   Longitude   Fix  Date       Time     Date Alt    Course Speed Card  Distance Course Card  Chars Sentences Checksum"));
    Serial.println(F("           (deg)      (deg)       Age                      Age  (m)    --- from GPS ----  ---- to London  ----  RX    RX        Fail"));
    Serial.println(F("----------------------------------------------------------------------------------------------------------------------------------------"));
}

void loop()
{
    static const double LONDON_LAT = 51.508131, LONDON_LON = -0.128002;
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

    unsigned long distanceKmToLondon =
        (unsigned long)TinyGPSPlus::distanceBetween(
            gps.location.lat(),
            gps.location.lng(),
            LONDON_LAT,
            LONDON_LON) / 1000;
    printInt(distanceKmToLondon, gps.location.isValid(), 9);

    double courseToLondon =
        TinyGPSPlus::courseTo(
            gps.location.lat(),
            gps.location.lng(),
            LONDON_LAT,
            LONDON_LON);

    printFloat(courseToLondon, gps.location.isValid(), 7, 2);

    const char *cardinalToLondon = TinyGPSPlus::cardinal(courseToLondon);

    printStr(gps.location.isValid() ? cardinalToLondon : "*** ", 6);

    printInt(gps.charsProcessed(), true, 6);
    printInt(gps.sentencesWithFix(), true, 10);
    printInt(gps.failedChecksum(), true, 9);
    Serial.println();

    smartDelay(1000);

    if (millis() > 5000 && gps.charsProcessed() < 10) {
        Serial.println(F("No GPS data received: check wiring"));
        delay(500);
    }
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
