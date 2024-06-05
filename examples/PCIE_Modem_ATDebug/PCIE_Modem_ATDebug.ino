/**
 * @file      PCIE_Modem_ATDebug.ino
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
 * 
 */

#include <Arduino.h>

#if   defined(LILYGO_T_INTER_COM)
#define MODEM_RX_PIN                    35
#define MODEM_TX_PIN                    33
#define MODEM_PWRKEY_PIN                32
#elif defined(LILYGO_T_ETH_ELITE_ESP32S3)
#define MODEM_RX_PIN                    4
#define MODEM_TX_PIN                    6
#define MODEM_DTR_PIN                   5
#define MODEM_RI_PIN                    1
#define MODEM_PWRKEY_PIN                3
#else
#error "Not define modem pin"
#endif

#define SerialAT        Serial2

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

    Serial.println("Start modem .");
    // Turn on modem
    pinMode(MODEM_PWRKEY_PIN, OUTPUT);
    digitalWrite(MODEM_PWRKEY_PIN, LOW);
    delay(100);
    digitalWrite(MODEM_PWRKEY_PIN, HIGH);
    delay(100);
    digitalWrite(MODEM_PWRKEY_PIN, LOW);
    
    delay(6000);

    if (checkAutoBaud()) {
        Serial.println(F("***********************************************************"));
        Serial.println(F(" You can now send AT commands"));
        Serial.println(F(" Enter \"AT\" (without quotes), and you should see \"OK\""));
        Serial.println(F(" If it doesn't work, select \"Both NL & CR\" in Serial Monitor"));
        Serial.println(F(" DISCLAIMER: Entering AT commands without knowing what they do"));
        Serial.println(F(" can have undesired consequences..."));
        Serial.println(F("***********************************************************\n"));
    } else {
        Serial.println(F("***********************************************************"));
        Serial.println(F(" Failed to connect to the modem! Check the baud and try again."));
        Serial.println(F("***********************************************************\n"));
    }

}

void loop()
{
    while (SerialAT.available()) {
        Serial.write(SerialAT.read());
    }
    while (Serial.available()) {
        SerialAT.write(Serial.read());
    }
    delay(1);
}
