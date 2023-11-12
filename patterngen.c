#include <stdio.h>
#include <pigpio.h>
#include <stdlib.h>
#include <math.h>
#include <errno.h>
#include <ncurses.h>
#include "lasermain.h"
#include "hwcontrol.h"

#define POS_MOVE_UP_DOWN 50
#define POS_MOVE_LEFT_RIGHT 50

stLaserPos laserPosTransform = {0, 0};

void *userInput() {
    int userInputChar;

    initscr();      /* Initialize ncurses */
    cbreak();       /* Send characters to stdin without newline or EOF */
    noecho();       /* Dont echo back characters sent to stdin*/
    keypad(stdscr, TRUE);

    printw("End session with CTRL+C\n");
    
    while(true) {
        userInputChar = getch();        /* Retrieve character from stdin with lib ncurses */
        // printw("\rKEY: %d", userInputChar);
        switch(userInputChar)
		{	case KEY_UP:
				laserPosTransform.y = laserPosTransform.y + POS_MOVE_UP_DOWN;
                printw("\rMoving UP: %d              ", laserPosTransform.y);
				break;
			case KEY_DOWN:
				laserPosTransform.y = laserPosTransform.y - POS_MOVE_UP_DOWN;
                printw("\rMoving DOWN: %d              ", laserPosTransform.y);
				break;
			case KEY_RIGHT:
				laserPosTransform.x = laserPosTransform.x + POS_MOVE_LEFT_RIGHT;
                printw("\rMoving RIGHT: %d              ", laserPosTransform.x);
				break;
			case KEY_LEFT:
				laserPosTransform.x = laserPosTransform.x - POS_MOVE_LEFT_RIGHT;
                printw("\rMoving LEFT: %d              ", laserPosTransform.x);
				break;
			default:
				break;
		}        
    }

    endwin();
    printf("Ending!\n");

    return NULL;
}

int createSineWave(int spiHandle, double delaySecs) {
    stLaserPos laserPos = {0, 0};
    double t = 0;

    for (;;) {
        t += 0.1;
        laserPos.x = 512 + (int)(510*cos(t)) + laserPosTransform.x;
        laserPos.y = 512 + (int)(510*sin(t)) + laserPosTransform.y;

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
            laserPos.x = laserPos.x + laserPosTransform.x;
            laserPos.y = laserPos.y + laserPosTransform.y;

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