#ifndef HX_H
#define HX_H

#include "HX711.h"

void initHX711(byte dataPin, byte clockPin);
float readForceValue();

#endif // HX_H
