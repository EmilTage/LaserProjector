#include <stdio.h>
#include <pigpio.h>
#include <stdlib.h>
#include <math.h>
#include <signal.h>
#include "lasermain.h"
#include "patterngen.h"
#include "hwcontrol.h"

/* Catch CTRL+C and close application gracefully */
void sig_handler(int signo)
{
   if (signo == SIGINT) {
      printf("Received SIGINT\n");
      gpioSetMode(GPIO_LASER, PI_OUTPUT);
      gpioWrite(GPIO_LASER, PI_LOW);
      gpioTerminate();
      exit(0);
   }
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

   if (signal(SIGINT, sig_handler) == SIG_ERR)
      printf("\nCan't catch SIGINT\n");

   spiHandle = spiOpen(SPI_CHANNEL_CE0, SPI_BAUD, PI_SPI_FLAGS_MODE(0) | 
                                               PI_SPI_FLAGS_BITLEN(16));
   printf("SPI Baudrate: %d\n", SPI_BAUD);

   if (spiHandle < 0) {
      printf("Failed to initalise SPI! Returned: %d\n", spiHandle);
      return -1;
   }

   /* Everything is now setup, create pattern */
   if (createSineWave(spiHandle, delaySecs) < 0)
      return -1;
   
   spiClose(spiHandle);
   gpioTerminate();
}