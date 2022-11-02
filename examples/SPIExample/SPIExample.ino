#include <Arduino.h>
#include <SPI.h>



#define SPI_MOSI    14
#define SPI_MISO    15
#define SPI_SCK     15
#define SPI_CS      15

void setup()
{
    // Using SPI requires an explicit call
    SPI.begin(SPI_SCK, SPI_MISO, SPI_MOSI, SPI_CS);
    // do someing ..
}

void loop()
{

}