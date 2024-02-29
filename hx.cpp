#include "hx.h"
#include "logger.h" 

HX711 scale;

float calibration_factor = -139.944621;

void initHX711(byte dataPin, byte clockPin) {
  logDebug("Initializing Force Sensor...");

  scale.begin(dataPin, clockPin);

  scale.set_scale();
  scale.tare();
  long zero_factor = scale.read_average(10);

  logDebug("Zero factor: " + String(zero_factor) + "\n");
}

float readForceValue() {
  scale.set_scale(calibration_factor);
  return scale.get_units();
}
