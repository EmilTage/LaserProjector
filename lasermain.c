#include <stdio.h>
#include <pigpio.h>
#include <stdlib.h>
#include <math.h>

typedef struct {
    uint16_t x;
    uint16_t y;
} stLaserPos;

const uint8_t SPI_CHANNEL_CE0 = 0;
const int SPI_BAUD = 300000; //(PI_SPI_MAX_BAUD >> 6);

const uint8_t DAC_CONFIG_A = 0b0001;     // DAC_A
const uint8_t DAC_CONFIG_B = 0b1001;     // DAC_B

const uint8_t GPIO_SPI_CE0 = 8;
const uint8_t GPIO_SPI_MOSI = 10;
const uint8_t GPIO_SPI_SCLK = 11;
const uint8_t GPIO_PWR_TO_GALVO = 17;
const uint8_t GPIO_LASER = 27;
const double DEFAULT_DELAY = 0.01;

/*

   Receives two 10-bit values (0-1023)
   One for X position of Laser and one for Y.
   Sends via SPI to DAC to set voltage level.

*/
int setDacVoltage(int spiHandle, stLaserPos laserPos) {
   const uint8_t bufSize = 2;
   char out_bufa[2];
   char out_bufb[2];

   if (laserPos.x > 1023)
      laserPos.x = 1023;
   if (laserPos.y > 1023)
      laserPos.y = 1023;
   if (laserPos.x < 0)
      laserPos.x = 0;
   if (laserPos.y < 0)
      laserPos.y = 0;

   out_bufa[0] = (DAC_CONFIG_A << 4) | ((char)(laserPos.x >> 6));
   out_bufa[1] = ((char)(laserPos.x << 2)) & 0xff;
   
   out_bufb[0] = (DAC_CONFIG_B << 4) | ((char)(laserPos.y >> 6));
   out_bufb[1] = ((char)(laserPos.y << 2)) & 0xff;

   if ((spiWrite(spiHandle, out_bufa, bufSize) != bufSize) |
       (spiWrite(spiHandle, out_bufb, bufSize) != bufSize)) {
      
      printf("Failed to write bytes to SPI\n");
      return -1;
   }

   return 0;
}

int createSineWave(int spiHandle, double delaySecs) {
   stLaserPos laserPos = {0, 0};
   double t = 0;

   for (;;) {
      t += 0.1;
      laserPos.x = 512 + (int)(510*sin(t));
      laserPos.y = 512 + (int)(510*sin(t));

      // printf("laserPos.x : %d    laserPos.y : %d\n", laserPos.x, laserPos.y);

      if (setDacVoltage(spiHandle, laserPos))
         return -1;

      time_sleep(delaySecs);

      if (t > 4096) 
         t = 0.0;
   }

   return 0;
}

void configureCtrlPins() {
   gpioSetMode(GPIO_PWR_TO_GALVO, PI_OUTPUT);
   gpioSetMode(GPIO_LASER, PI_OUTPUT);

   gpioWrite(GPIO_PWR_TO_GALVO, PI_HIGH);
   gpioWrite(GPIO_LASER, PI_HIGH);
}

int main(int argc, char *argv[])
{
   int status = 0;
   int spiHandle = 0;
   double delaySecs = DEFAULT_DELAY;

   if(argc > 1)
      delaySecs = atof(argv[1]);

   printf("Setting delay: %lf\n", delaySecs);

   status = gpioInitialise();
   if (status < 0) {
      printf("Failed to initalise! Returned: %d\n", status);
      return -1;
   }

   configureCtrlPins();

   spiHandle = spiOpen(SPI_CHANNEL_CE0, SPI_BAUD, PI_SPI_FLAGS_MODE(0) | 
                                               PI_SPI_FLAGS_BITLEN(16));
   printf("SPI Baudrate: %d\n", SPI_BAUD);

   if (spiHandle < 0) {
      printf("Failed to initalise SPI! Returned: %d\n", spiHandle);
      return -1;
   }

   if (createSineWave(spiHandle, delaySecs) < 0)
      return -1;
   
   spiClose(spiHandle);
   gpioTerminate();
}