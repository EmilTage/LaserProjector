#ifndef HWCONTROL_H
#define HWCONTROL_H 1

#include "lasermain.h"

int setDacVoltage(int, stLaserPos);

void configureCtrlPins();

#endif