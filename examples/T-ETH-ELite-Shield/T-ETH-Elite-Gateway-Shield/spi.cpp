
#include <SPI.h>
#include <Arduino.h>
#include "loragw_spi.h"
#include "loragw_reg.h"

#define RADIO_CS_PIN                     41
#define SPI_MISO_PIN                     9
#define SPI_MOSI_PIN                     11
#define SPI_SCLK_PIN                     10
#define RADIO_RST_PIN                    2

#define READ_ACCESS     0x00
#define WRITE_ACCESS    0x80
#define ARRAY_SIZE(a) (sizeof(a) / sizeof((a)[0]))



/**
@brief LoRa concentrator SPI setup (configure I/O and peripherals)
@param spidev_path path to the SPI device to be used to connect to the SX1302
@param spi_target_ptr pointer on a generic pointer to SPI target (implementation dependant)
@return status of register operation (LGW_SPI_SUCCESS/LGW_SPI_ERROR)
*/

int lgw_spi_open(const char *spidev_path, void **spi_target_ptr)
{
    pinMode(RADIO_RST_PIN, OUTPUT);

    digitalWrite(RADIO_RST_PIN, HIGH); delay(100);

    digitalWrite(RADIO_RST_PIN, LOW); delay(100);

    // Using SPI requires an explicit call
    SPI.begin(SPI_SCLK_PIN, SPI_MISO_PIN, SPI_MOSI_PIN);

    pinMode(RADIO_CS_PIN, OUTPUT);

    *spi_target_ptr = &SPI;

    return LGW_SPI_SUCCESS;
}


/**
@brief LoRa concentrator SPI close
@param spi_target generic pointer to SPI target (implementation dependant)
@return status of register operation (LGW_SPI_SUCCESS/LGW_SPI_ERROR)
*/

int lgw_spi_close(void *spi_target)
{
    return LGW_SPI_SUCCESS;
}

/**
@brief LoRa concentrator SPI single-byte write
@param spi_target generic pointer to SPI target (implementation dependant)
@param address 7-bit register address
@param data data byte to write
@return status of register operation (LGW_SPI_SUCCESS/LGW_SPI_ERROR)
*/
int lgw_spi_w(void *spi_target, uint8_t spi_mux_target, uint16_t address, uint8_t data)
{
    // printf("lgw_spi_w -> spi_mux_target: %u address: %X data[0]: %X  size: %u\n", spi_mux_target, address, data);

    uint8_t out_buf[4];
    uint8_t command_size;

    /* prepare frame to be sent */
    out_buf[0] = spi_mux_target;
    out_buf[1] = WRITE_ACCESS | ((address >> 8) & 0x7F);
    out_buf[2] =                ((address >> 0) & 0xFF);
    out_buf[3] = data;
    command_size = 4;

    digitalWrite(RADIO_CS_PIN, LOW);
    SPI.beginTransaction( SPISettings(8E6, MSBFIRST, SPI_MODE0));
    SPI.transfer(out_buf, command_size);
    SPI.endTransaction();
    digitalWrite(RADIO_CS_PIN, HIGH);

    return LGW_SPI_SUCCESS;
}


/**
@brief LoRa concentrator SPI single-byte read
@param spi_target generic pointer to SPI target (implementation dependant)
@param address 7-bit register address
@param data data byte to write
@return status of register operation (LGW_SPI_SUCCESS/LGW_SPI_ERROR)
*/
int lgw_spi_r(void *spi_target, uint8_t spi_mux_target, uint16_t address, uint8_t *data)
{
    uint8_t out_buf[5];
    uint8_t command_size;
    uint8_t in_buf[ARRAY_SIZE(out_buf)];

    /* prepare frame to be sent */
    out_buf[0] = spi_mux_target;
    out_buf[1] = READ_ACCESS | ((address >> 8) & 0x7F);
    out_buf[2] =               ((address >> 0) & 0xFF);
    out_buf[3] = 0x00;
    out_buf[4] = 0x00;
    command_size = 5;

    SPI.beginTransaction( SPISettings());
    digitalWrite(RADIO_CS_PIN, LOW);
    SPI.transfer(out_buf, command_size);
    SPI.endTransaction();
    digitalWrite(RADIO_CS_PIN, HIGH);
    *data = out_buf[command_size - 1];


    // printf("lgw_spi_r -> spi_mux_target: %u address: %X data[0]: %X  size: %u\n", spi_mux_target, address, data[0]);

    return LGW_SPI_SUCCESS;
}


/**
@brief LoRa concentrator SPI burst (multiple-byte) write
@param spi_target generic pointer to SPI target (implementation dependant)
@param address 7-bit register address
@param data pointer to byte array that will be sent to the LoRa concentrator
@param size size of the transfer, in byte(s)
@return status of register operation (LGW_SPI_SUCCESS/LGW_SPI_ERROR)
*/
int lgw_spi_wb(void *spi_target, uint8_t spi_mux_target, uint16_t address, const uint8_t *data, uint16_t size)
{

    // printf("lgw_spi_wb -> spi_mux_target: %u address: %X data[0]: %X  size: %u\n", spi_mux_target, address, data[0], size);

    int spi_device;
    uint8_t command[3];
    uint8_t command_size;
    // struct spi_ioc_transfer k[2];
    int size_to_do, chunk_size, offset;
    int byte_transferred = 0;
    int i;

    // /* check input parameters */
    CHECK_NULL(data);
    if (size == 0) {
        DEBUG_MSG("ERROR: BURST OF NULL LENGTH\n");
        return LGW_SPI_ERROR;
    }

    // //spi_device = *(int *)spi_target; /* must check that spi_target is not null beforehand */

    // /* prepare command byte */
    command[0] = spi_mux_target;
    command[1] = WRITE_ACCESS | ((address >> 8) & 0x7F);
    command[2] =                ((address >> 0) & 0xFF);
    command_size = 3;
    size_to_do = size;

    // /* I/O transaction */
    // memset(&k, 0, sizeof(k)); /* clear k */
    // k[0].tx_buf = (unsigned long) &command[0];
    // k[0].len = command_size;
    // k[0].cs_change = 0;
    // k[1].cs_change = 0;

    for (i = 0; size_to_do > 0; ++i) {
        chunk_size = (size_to_do < LGW_BURST_CHUNK) ? size_to_do : LGW_BURST_CHUNK;
        offset = i * LGW_BURST_CHUNK;
        //     k[1].tx_buf = (unsigned long)(data + offset);
        //     k[1].len = chunk_size;
        //     byte_transferred += (ioctl(spi_device, SPI_IOC_MESSAGE(2), &k) - k[0].len );
        //     DEBUG_PRINTF("BURST WRITE: to trans %d # chunk %d # transferred %d \n", size_to_do, chunk_size, byte_transferred);
        size_to_do -= chunk_size; /* subtract the quantity of data already transferred */
    }

    // /* determine return code */
    if (byte_transferred != size) {
        DEBUG_MSG("ERROR: SPI BURST WRITE FAILURE\n");
        return LGW_SPI_ERROR;
    } else {
        DEBUG_MSG("Note: SPI burst write success\n");
        return LGW_SPI_SUCCESS;
    }

    return LGW_SPI_SUCCESS;
}

/**
@brief LoRa concentrator SPI burst (multiple-byte) read
@param spi_target generic pointer to SPI target (implementation dependant)
@param address 7-bit register address
@param data pointer to byte array that will be written from the LoRa concentrator
@param size size of the transfer, in byte(s)
@return status of register operation (LGW_SPI_SUCCESS/LGW_SPI_ERROR)
*/
int lgw_spi_rb(void *spi_target, uint8_t spi_mux_target, uint16_t address, uint8_t *data, uint16_t size)
{
    int spi_device;
    uint8_t command[4];
    uint8_t command_size;
    // struct spi_ioc_transfer k[2];
    int size_to_do, chunk_size, offset;
    int byte_transferred = 0;
    int i;

    // printf("lgw_spi_rb -> spi_mux_target: %u address: %X data[0]: %X  size: %u\n", spi_mux_target, address, data[0], size);

    // /* check input parameters */
    CHECK_NULL(data);
    if (size == 0) {
        DEBUG_MSG("ERROR: BURST OF NULL LENGTH\n");
        return LGW_SPI_ERROR;
    }

    // spi_device = *(int *)spi_target; /* must check that spi_target is not null beforehand */

    // /* prepare command byte */
    command[0] = spi_mux_target;
    command[1] = READ_ACCESS | ((address >> 8) & 0x7F);
    command[2] =               ((address >> 0) & 0xFF);
    command[3] = 0x00;
    command_size = 4;
    size_to_do = size;

    // /* I/O transaction */
    // memset(&k, 0, sizeof(k)); /* clear k */
    // k[0].tx_buf = (unsigned long) &command[0];
    // k[0].len = command_size;
    // k[0].cs_change = 0;
    // k[1].cs_change = 0;
    for (i = 0; size_to_do > 0; ++i) {
        chunk_size = (size_to_do < LGW_BURST_CHUNK) ? size_to_do : LGW_BURST_CHUNK;
        offset = i * LGW_BURST_CHUNK;
        //     k[1].rx_buf = (unsigned long)(data + offset);
        //     k[1].len = chunk_size;
        //     byte_transferred += (ioctl(spi_device, SPI_IOC_MESSAGE(2), &k) - k[0].len );
        //     DEBUG_PRINTF("BURST READ: to trans %d # chunk %d # transferred %d \n", size_to_do, chunk_size, byte_transferred);
        size_to_do -= chunk_size;  /* subtract the quantity of data already transferred */
    }

    /* determine return code */
    if (byte_transferred != size) {
        DEBUG_MSG("ERROR: SPI BURST READ FAILURE\n");
        return LGW_SPI_ERROR;
    } else {
        DEBUG_MSG("Note: SPI burst read success\n");
        return LGW_SPI_SUCCESS;
    }
    return LGW_SPI_SUCCESS;
}

