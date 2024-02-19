#include "logger.h"

void logDebug(const String &message) {
  #ifdef DEBUG
  Serial.println(message);
  #endif
}