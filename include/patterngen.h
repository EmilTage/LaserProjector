#ifndef PATTERNGEN_H
#define PATTERNGEN_H 1

#include <termios.h>

int createSineWave(int);
int createPatternFromFile(char *filename, int spiHandle);
void *userInput();

/*    Global variables    */
extern struct termios oldt;

#endif