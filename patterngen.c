#include <stdio.h>
#include <pigpio.h>
#include <stdlib.h>
#include <math.h>
#include <errno.h>
#include "lasermain.h"
#include "hwcontrol.h"

int createSineWave(int spiHandle, double delaySecs) {
   stLaserPos laserPos = {0, 0};
   double t = 0;

   for (;;) {
      t += 0.1;
      laserPos.x = 512 + (int)(510*cos(t));
      laserPos.y = 512 + (int)(510*sin(t));

      // printf("laserPos.x : %d    laserPos.y : %d\n", laserPos.x, laserPos.y);

      if (setDacVoltage(spiHandle, laserPos) < 0)
         return -1;

      time_sleep(delaySecs);

      if (t > 4096) 
         t = 0.0;
   }

   return 0;
}

int createPatternFromFile(char *filename, int spiHandle, double delaySecs) {
    FILE *pointMapFD;
    stLaserPos laserPos;
    
    if ((pointMapFD = fopen(filename, "r")) == NULL) {
        printf("Failed to open file: %s, errno: %d\n", filename, errno);
        return -1;
    }

    for (;;)
    {
        while (fscanf(pointMapFD, "%hd %hd", &laserPos.x, &laserPos.y) == 2) {
            // printf("laserPos.x: %hd,    laserPos.y: %hd\n", laserPos.x, laserPos.y);
            if (setDacVoltage(spiHandle, laserPos) < 0)
                return -1;

            time_sleep(delaySecs);
        }
        rewind(pointMapFD);         /* Reset the file position indicator to the beginning */
    }
    

	if (fclose(pointMapFD) != 0) {
        printf("Failed to close file: %s, errno: %d\n", filename, errno);
        return -1;
    }

    return 0;
}