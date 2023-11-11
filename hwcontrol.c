#include <stdio.h>
#include <pigpio.h>
#include <stdlib.h>
#include "lasermain.h"
#include "patterngen.h"
#include "hwcontrol.h"

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

void configureCtrlPins() {
   gpioSetMode(GPIO_PWR_TO_GALVO, PI_OUTPUT);
   gpioSetMode(GPIO_LASER, PI_OUTPUT);

   gpioWrite(GPIO_PWR_TO_GALVO, PI_HIGH);
   gpioWrite(GPIO_LASER, PI_HIGH);
}