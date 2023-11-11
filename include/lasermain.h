#ifndef LASERMAIN_H
#define LASERMAIN_H 1

#include <stdlib.h>

typedef struct {
    uint16_t x;
    uint16_t y;
} stLaserPos;

#define SPI_CHANNEL_CE0     0
#define SPI_BAUD            300000
#define DAC_CONFIG_A        0b0001     // DAC_A
#define DAC_CONFIG_B        0b1001     // DAC_B
#define GPIO_SPI_CE0        8
#define GPIO_SPI_MOSI       10
#define GPIO_SPI_SCLK       11
#define GPIO_PWR_TO_GALVO   17
#define GPIO_LASER          27
#define DEFAULT_DELAY       0.01

#endif