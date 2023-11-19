#include <stdio.h>
#include <pigpio.h>
#include <stdlib.h>
#include <math.h>
#include <errno.h>
#include <ncurses.h>
#include "lasermain.h"
#include "hwcontrol.h"

#define KEY_Q 113
#define KEY_E 101
#define KEY_MINUS 45
#define KEY_PLUS 43
#define KEY_S 115
#define KEY_D 100

#define POS_MOVE_UP_DOWN 50
#define POS_MOVE_LEFT_RIGHT 50
#define SCALE_FACTOR 0.1
#define SPEED_FACTOR 2

#define IMAGE_CENTER 512

stLaserPos laserPosTransform = {0, 0};
double rotationTheta = 0.0;
double scale = 1.0;

int transform(stLaserPos *calcLaserPos) {
    stLaserPos transformedLaserPos = *calcLaserPos;
    float x_origin, y_origin;

    /* ############# Move image ############# */
    transformedLaserPos.x = transformedLaserPos.x + laserPosTransform.x;
    transformedLaserPos.y = transformedLaserPos.y + laserPosTransform.y;

    // translate point back to origin:
    transformedLaserPos.x -= IMAGE_CENTER;
    transformedLaserPos.y -= IMAGE_CENTER;

    /* ############# Rotate image ############# */
    x_origin = transformedLaserPos.x * cos(rotationTheta) - transformedLaserPos.y * sin(rotationTheta);
    y_origin = transformedLaserPos.x * sin(rotationTheta) + transformedLaserPos.y * cos(rotationTheta);

    /* ############# Scale image ############# */
    x_origin = x_origin * scale;
    y_origin = y_origin * scale;

    // translate point back:
    transformedLaserPos.x = x_origin + IMAGE_CENTER;
    transformedLaserPos.y = y_origin + IMAGE_CENTER;

    *calcLaserPos = transformedLaserPos;

    return 0;
}

void *userInput() {
    int userInputChar;

    initscr();      /* Initialize ncurses */
    cbreak();       /* Send characters to stdin without newline or EOF */
    noecho();       /* Dont echo back characters sent to stdin*/
    keypad(stdscr, TRUE);

    printw("End session with CTRL+C\n");
    
    while(true) {
        userInputChar = getch();        /* Retrieve character from stdin with lib ncurses */
        printw("\rKEY: %d", userInputChar);
        switch(userInputChar)
		{	case KEY_UP:
				laserPosTransform.y = laserPosTransform.y + POS_MOVE_UP_DOWN;
				break;
			case KEY_DOWN:
				laserPosTransform.y = laserPosTransform.y - POS_MOVE_UP_DOWN;
				break;
			case KEY_RIGHT:
				laserPosTransform.x = laserPosTransform.x + POS_MOVE_LEFT_RIGHT;
				break;
			case KEY_LEFT:
				laserPosTransform.x = laserPosTransform.x - POS_MOVE_LEFT_RIGHT;
				break;
			case KEY_Q:
                rotationTheta = rotationTheta - M_PI/10;
				break;
			case KEY_E:
                rotationTheta = rotationTheta + M_PI/10;
				break;
			case KEY_MINUS:
                scale = scale - SCALE_FACTOR;
				break;
			case KEY_PLUS:
                scale = scale + SCALE_FACTOR;
				break;
			case KEY_S:
                delaySecs = delaySecs / SPEED_FACTOR;
				break;
			case KEY_D:
                delaySecs = delaySecs * SPEED_FACTOR;
				break;
			default:
				break;
		}        
    }

    endwin();
    printf("Ending!\n");

    return NULL;
}

int createSineWave(int spiHandle) {
    stLaserPos laserPos = {0, 0};
    double t = 0;

    for (;;) {
        t += 0.1;
        laserPos.x = 512 + (int)(510*cos(t));
        laserPos.y = 512 + (int)(510*sin(t));

        transform(&laserPos);

        if (setDacVoltage(spiHandle, laserPos) < 0)
            return -1;

        time_sleep(delaySecs);

        if (t > 4096) 
            t = 0.0;
   }

   return 0;
}

int createPatternFromFile(char *filename, int spiHandle) {
    FILE *pointMapFD;
    stLaserPos laserPos;
    
    if ((pointMapFD = fopen(filename, "r")) == NULL) {
        printf("Failed to open file: %s, errno: %d\n", filename, errno);
        return -1;
    }

    for (;;)
    {
        while (fscanf(pointMapFD, "%hd %hd", &laserPos.x, &laserPos.y) == 2) {
            laserPos.x = laserPos.x;
            laserPos.y = laserPos.y;

            transform(&laserPos);

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