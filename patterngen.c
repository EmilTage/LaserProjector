#include <stdio.h>
#include <pigpio.h>
#include <stdlib.h>
#include <math.h>
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

      if (setDacVoltage(spiHandle, laserPos))
         return -1;

      time_sleep(delaySecs);

      if (t > 4096) 
         t = 0.0;
   }

   return 0;
}