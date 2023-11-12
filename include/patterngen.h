#ifndef PATTERNGEN_H
#define PATTERNGEN_H 1

#include <termios.h>

int createSineWave(int, double);
int createPatternFromFile(char *filename, int spiHandle, double delaySecs);
void *userInput();

/*    Global variables    */
extern struct termios oldt;

#endif