#include <stdio.h>
#include <pigpio.h>
#include <stdlib.h>
#include <signal.h>
#include <getopt.h>
#include <string.h>
#include "lasermain.h"
#include "patterngen.h"
#include "hwcontrol.h"

static struct option long_options[] = {
   {"delay",   required_argument, 0,  'd' },
   {"file",    required_argument, 0,  'f' },
   {"sine",    no_argument,       0,  's' },
   {0,         0,                 0,  0   }
};

enum patterns {
   PATTERN_SINEWAVE = 1,
   PATTERN_FILE
};

void cleanup() {
   gpioSetMode(GPIO_LASER, PI_OUTPUT);
   gpioWrite(GPIO_LASER, PI_LOW);
   gpioTerminate();
}

/* Catch CTRL+C and close application gracefully */
void sig_handler(int signo)
{
   if (signo == SIGINT) {
      printf("Received SIGINT\n");
      cleanup();
      exit(0);
   }
}

int main(int argc, char *argv[])
{
   int status        = 0;
   int spiHandle     = 0;
   int opt           = 0;
   int long_index    = 0;
   int pattern       = PATTERN_SINEWAVE;
   double delaySecs  = DEFAULT_DELAY;
   char *filename;

   while ((opt = getopt_long(argc, argv,"d:f:s", 
                  long_options, &long_index )) != -1) {
      switch (opt) {
            case 'd' :
               delaySecs = atof(optarg);
               break;
            case 's' : 
               pattern = PATTERN_SINEWAVE;
               break;
            case 'f' : 
               pattern = PATTERN_FILE;
               filename = optarg; 
               break;
            default:
               printf("-d|--delay : 0.001; -s|--sine; -f|--file : heart.txt\n");
               exit(-1);
      }
   }

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

   if (spiHandle < 0) {
      printf("Failed to initalise SPI! Returned: %d\n", spiHandle);
      return -1;
   }

   /* Everything is now setup, create pattern */
   switch (pattern) {
      case PATTERN_SINEWAVE :
         if (createSineWave(spiHandle, delaySecs) < 0)
            return -1;
         break;
      case PATTERN_FILE : 
         createPatternFromFile(filename, spiHandle, delaySecs);
         break;
      default: exit(-1);
   }
   
   spiClose(spiHandle);
   cleanup();
}