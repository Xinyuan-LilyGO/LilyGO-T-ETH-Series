
#pragma once

// Product Link : https://www.lilygo.cc/products/t-internet-poe
// #define LILYGO_T_INTERNET_POE

// Product Link : https://www.lilygo.cc/products/t-poe-pro
// #define LILYGO_T_ETH_POE_PRO

// Product Link : https://www.lilygo.cc/products/t-internet-com
// #define LILYGO_T_INTER_COM

// Product Link : https://www.lilygo.cc/products/t-eth-lite?variant=43120880746677
// #define LILYGO_T_ETH_LITE_ESP32

// Product Link : https://www.lilygo.cc/products/t-eth-lite?variant=43120880779445
#define LILYGO_T_ETH_LITE_ESP32S3

// Product Link : N.A
// #define LILYGO_T_ETH_ELITE_ESP32S3

#if   defined(LILYGO_T_INTERNET_POE)
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
#define MODEM_RX_PIN                    35
#define MODEM_TX_PIN                    33
#define MODEM_PWRKEY_PIN                32
#define RGBLED_PIN                      12

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
#elif defined(LILYGO_T_ETH_ELITE_ESP32S3)

#define ETH_MISO_PIN                     47
#define ETH_MOSI_PIN                     21
#define ETH_SCLK_PIN                     48
#define ETH_CS_PIN                       45
#define ETH_INT_PIN                      14
#define ETH_RST_PIN                      -1
#define ETH_ADDR                         1

#define SPI_MISO_PIN                     9
#define SPI_MOSI_PIN                     11
#define SPI_SCLK_PIN                     10

#define SD_MISO_PIN                     SPI_MISO_PIN
#define SD_MOSI_PIN                     SPI_MOSI_PIN
#define SD_SCLK_PIN                     SPI_SCLK_PIN
#define SD_CS_PIN                       12

#define I2C_SDA_PIN                     17
#define I2C_SCL_PIN                     18

#define RADIO_MISO_PIN                  SPI_MISO_PIN
#define RADIO_MOSI_PIN                  SPI_MOSI_PIN
#define RADIO_SCLK_PIN                  SPI_SCLK_PIN
#define RADIO_CS_PIN                    40
#define RADIO_RST_PIN                   46
// #define RADIO_DIO1_PIN                  16
#define RADIO_IRQ_PIN                   8
#define RADIO_BUSY_PIN                  16

#define ADC_BUTTONS_PIN                 7

#define MODEM_RX_PIN                    4
#define MODEM_TX_PIN                    6
#define MODEM_DTR_PIN                   5
#define MODEM_RI_PIN                    1
#define MODEM_PWRKEY_PIN                3

#define GPS_RX_PIN                      39
#define GPS_TX_PIN                      42

#define LED_PIN                         38

#else
#error "Use ArduinoIDE, please open the macro definition corresponding to the board above <utilities.h>"
#endif









