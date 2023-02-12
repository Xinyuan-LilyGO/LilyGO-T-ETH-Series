#include <Arduino.h>
#include <SPI.h>


#define SPI_MOSI    4       //OUTPUT
#define SPI_MISO    35      //INPUT
#define SPI_SCK     33      //OUTPUT
#define SPI_CS      16      //OUTPUT

// IO35,39,34,36 can only be used for input and cannot be set as output

void setup()
{
    // Using SPI requires an explicit call
    SPI.begin(SPI_SCK, SPI_MISO, SPI_MOSI, SPI_CS);
    // do someing ..
}

void loop()
{

}