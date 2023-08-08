
#pragma once

// #define LILYGO_T_ETH_POE
// #define LILYGO_T_ETH_POE_PRO
// #define LILYGO_T_INTER_COM
// #define LILYGO_T_ETH_LITE_ESP32

#ifndef LILYGO_T_ETH_LITE_ESP32S3
#define LILYGO_T_ETH_LITE_ESP32S3
#endif

#if   defined(LILYGO_T_ETH_POE)
#define ETH_CLK_MODE                    ETH_CLOCK_GPIO17_OUT
#define ETH_ADDR                        0
#define ETH_TYPE                        ETH_PHY_LAN8720
#define ETH_RESET_PIN                   5
#define ETH_MDC_PIN                     23
#define ETH_MDIO_PIN                    18
#define SD_MISO_PIN                     2
#define SD_MOSI_PIN                     15
#define SD_SCLK_PIN                     14
#define SD_CS_PIN                       13

#elif defined(LILYGO_T_ETH_POE_PRO)
#define ETH_TYPE                        ETH_PHY_LAN8720
#define ETH_ADDR                        0
#define ETH_CLK_MODE                    ETH_CLOCK_GPIO0_OUT
#define ETH_RESET_PIN                   5
#define ETH_MDC_PIN                     23
#define ETH_MDIO_PIN                    18
#define SD_MISO_PIN                     12
#define SD_MOSI_PIN                     13
#define SD_SCLK_PIN                     14
#define SD_CS_PIN                       15
#define TFT_DC                          2
#define RS485_TX                        32
#define RS485_RX                        33

#elif defined(LILYGO_T_INTER_COM)
#define ETH_TYPE                        ETH_PHY_LAN8720
#define ETH_ADDR                        0
#define ETH_CLK_MODE                    ETH_CLOCK_GPIO0_OUT
#define ETH_RESET_PIN                   4
#define ETH_MDC_PIN                     23
#define ETH_MDIO_PIN                    18
#define SD_MISO_PIN                     2
#define SD_MOSI_PIN                     15
#define SD_SCLK_PIN                     14
#define SD_CS_PIN                       13

#elif defined(LILYGO_T_ETH_LITE_ESP32)
#define ETH_TYPE                        ETH_PHY_RTL8201
#define ETH_ADDR                        0
#define ETH_CLK_MODE                    ETH_CLOCK_GPIO0_IN
#define ETH_RESET_PIN                   -1
#define ETH_MDC_PIN                     23
#define ETH_POWER_PIN                   12
#define ETH_MDIO_PIN                    18
#define SD_MISO_PIN                     34
#define SD_MOSI_PIN                     13
#define SD_SCLK_PIN                     14
#define SD_CS_PIN                       5

#elif defined(LILYGO_T_ETH_LITE_ESP32S3)
#define ETH_MISO_PIN                    11
#define ETH_MOSI_PIN                    12
#define ETH_SCLK_PIN                    10
#define ETH_CS_PIN                      9
#define ETH_INT_PIN                     13
#define ETH_RST_PIN                     14
#define ETH_ADDR                        1
#define SD_MISO_PIN                     5
#define SD_MOSI_PIN                     6
#define SD_SCLK_PIN                     7
#define SD_CS_PIN                       42


#define IR_FILTER_NUM                   46

#else
#error "Use ArduinoIDE, please open the macro definition corresponding to the board above <utilities.h>"
#endif









