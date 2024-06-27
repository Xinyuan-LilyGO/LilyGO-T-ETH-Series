/**
 * @file      config.h
 * @author    Lewis He (lewishe@outlook.com)
 * @license   MIT
 * @copyright Copyright (c) 2024  ShenZhen XinYuan Electronic Technology Co., Ltd
 * @date      2024-06-22
 * 
 */
#include <SD.h>
#include <RadioLib.h>
#include <U8g2lib.h>
#include <Wire.h>
#include "utilities.h"

// Please select the LoRa model you need to use in config.h
// #define USING_SX1262
// #define USING_LR1121
// #define USING_SX1280
// #define USING_SX1276
// #define USING_SX1276_TCXO
// #define USING_SX1280PA


#define DISPLAY_MODEL               U8G2_SSD1306_128X64_NONAME_F_HW_I2C
#define DISPLAY_ADDR                0x3C
#define U8G2_HOR_ALIGN_CENTER(t)    ((u8g2->getDisplayWidth() -  (u8g2->getUTF8Width(t))) / 2)
#define U8G2_HOR_ALIGN_RIGHT(t)     ( u8g2->getDisplayWidth()  -  u8g2->getUTF8Width(t))


DISPLAY_MODEL *u8g2 = NULL;

#if     defined(USING_SX1276)
#define CONFIG_RADIO_FREQ           850.0
#define CONFIG_RADIO_OUTPUT_POWER   17
#define CONFIG_RADIO_BW             125.0
SX1276 radio = new Module(RADIO_CS_PIN, RADIO_IRQ_PIN, RADIO_RST_PIN, RADIO_BUSY_PIN);


#elif   defined(USING_SX1276_TCXO)
#define CONFIG_RADIO_FREQ           850.0
#define CONFIG_RADIO_OUTPUT_POWER   17
#define CONFIG_RADIO_BW             125.0
#define TCXO_ENABLE_PIN             38
SX1276 radio = new Module(RADIO_CS_PIN, RADIO_IRQ_PIN, RADIO_RST_PIN, RADIO_BUSY_PIN);


#elif   defined(USING_SX1278)
#define CONFIG_RADIO_FREQ           433.0
#define CONFIG_RADIO_OUTPUT_POWER   17
#define CONFIG_RADIO_BW             125.0
SX1278 radio = new Module(RADIO_CS_PIN, RADIO_IRQ_PIN, RADIO_RST_PIN, RADIO_BUSY_PIN);

#elif   defined(USING_SX1262)
#define CONFIG_RADIO_FREQ           850.0
#define CONFIG_RADIO_OUTPUT_POWER   22
#define CONFIG_RADIO_BW             125.0

SX1262 radio = new Module(RADIO_CS_PIN, RADIO_IRQ_PIN, RADIO_RST_PIN, RADIO_BUSY_PIN);

#elif   defined(USING_SX1280)
#define CONFIG_RADIO_FREQ           2450.0
#define CONFIG_RADIO_OUTPUT_POWER   13
#define CONFIG_RADIO_BW             203.125
SX1280 radio = new Module(RADIO_CS_PIN, RADIO_IRQ_PIN, RADIO_RST_PIN, RADIO_BUSY_PIN);

#define RADIO_RX_PIN    13
#define RADIO_TX_PIN    38

#elif defined(USING_SX1280PA)
#define CONFIG_RADIO_FREQ           2450.0
// PA Version power range : -18 ~ 3dBm
#define CONFIG_RADIO_OUTPUT_POWER  3
#define CONFIG_RADIO_BW             203.125
SX1280 radio = new Module(RADIO_CS_PIN, RADIO_IRQ_PIN, RADIO_RST_PIN, RADIO_BUSY_PIN);


#define RADIO_RX_PIN    13
#define RADIO_TX_PIN    38


#elif   defined(USING_SX1268)
#define CONFIG_RADIO_FREQ           433.0
#define CONFIG_RADIO_OUTPUT_POWER   22
#define CONFIG_RADIO_BW             125.0
SX1268 radio = new Module(RADIO_CS_PIN, RADIO_IRQ_PIN, RADIO_RST_PIN, RADIO_BUSY_PIN);

#elif   defined(USING_LR1121)

// The maximum power of LR1121 2.4G band can only be set to 13 dBm
#define CONFIG_RADIO_FREQ           2450.0
#define CONFIG_RADIO_OUTPUT_POWER   13
#define CONFIG_RADIO_BW             125.0

// The maximum power of LR1121 Sub 1G band can only be set to 22 dBm
#define CONFIG_RADIO_FREQ1           850.0
#define CONFIG_RADIO_OUTPUT_POWER1   22
#define CONFIG_RADIO_BW1             125.0

LR1121 radio = new Module(RADIO_CS_PIN, RADIO_IRQ_PIN, RADIO_RST_PIN, RADIO_BUSY_PIN);

#else

#error "Please select the LoRa model you need to use in config.h !"

#endif




