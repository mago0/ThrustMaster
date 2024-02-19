#include <Arduino.h>
#include <SPI.h>
#include <SD.h>
#include "config.h"
#include "logger.h"
#include "hx.h"

File dataFile;

bool start = false;
const unsigned long debounceDelay = 50;
static unsigned long lastInterruptTime = 0;

enum TestState {
  WAITING,
  RUNNING,
  FINISHED
};
TestState testState = WAITING;

struct Measurement {
  float time;
  float force;
};

const int maxMeasurements = 1000;
Measurement measurements[maxMeasurements];
int measurementCount = 0;

unsigned long pyroStartTime = 0;
unsigned long testStartTime = 0;
float runTime = 0.0; // used in RUNNING state

void IRAM_ATTR handleButtonPress() {
  unsigned long interruptTime = millis();

  if (interruptTime - lastInterruptTime > debounceDelay) {
    start = true;
    // Remember the time of the (debounced) interrupt
    lastInterruptTime = interruptTime;
  }
}

void setup() {
  #ifdef DEBUG
  Serial.begin(115200);
  #endif

  initSDCard();
  initHX711(PIN_ADC_DT, PIN_ADC_SCK);

  pinMode(PIN_BTN, INPUT_PULLUP);
  pinMode(PIN_LED, OUTPUT);
  pinMode(PIN_PYRO, OUTPUT);

  attachInterrupt(digitalPinToInterrupt(PIN_BTN), handleButtonPress, FALLING);

  digitalWrite(PIN_LED, HIGH);
}

void loop() {
  if (pyroStartTime > 0 && millis() - pyroStartTime > PYRO_DURATION) {
    digitalWrite(PIN_PYRO, LOW);
    pyroStartTime = 0; // Reset pyroStartTime to avoid multiple LOW signals
  }

  switch (testState) {
    case WAITING:
      if (start) {
        startNotice();
        digitalWrite(PIN_PYRO, HIGH);
        pyroStartTime = millis();
        testStartTime = millis();
        testState = RUNNING;
      }
      break;
    
    case RUNNING:
      runTime = ((float)millis() - testStartTime) / 1000;
      if (runTime < TEST_DURATION && measurementCount < maxMeasurements) {
        float force = readForceValue();

        measurements[measurementCount].time = runTime;
        measurements[measurementCount].force = force;
        measurementCount++;

        logDebug("Time: " + String(runTime) + ", Force: " + String(force));
      } else {
        logDebug("Stopping test.");
        testState = FINISHED;
      }
      break;

    case FINISHED:
      logDebug("Test sequence finished");
      dataFile = SD.open(FILENAME, FILE_WRITE);
      if (dataFile) {
        dataFile.println("Time, Force");
        for (int i = 0; i < measurementCount; i++) {
          dataFile.print(measurements[i].time);
          dataFile.print(",");
          dataFile.println(measurements[i].force);
        }
        dataFile.close();
        logDebug("Data written to SD card.");
      } else {
        logDebug("Error opening " + String(FILENAME));
      }
      while (true) {
        endNotice();
      }
      break;
  }
}

void startNotice() {
  for (int i = COUNTDOWN_TIME; i > 0; i--) {
    digitalWrite(PIN_LED, HIGH);
    #ifdef SPEAKER_ENABLE
    tone(PIN_SPEAKER, 1000); // 1000 Hz tone
    #endif
    delay(500); // Half a second on

    digitalWrite(PIN_LED, LOW);
    #ifdef SPEAKER_ENABLE
    noTone(PIN_SPEAKER);
    #endif
    delay(500); // Half a second off
  }
  digitalWrite(PIN_LED, HIGH);
}

void endNotice() {
  for (int i = 0; i < 3; i++) {
    digitalWrite(PIN_LED, HIGH);
    #ifdef SPEAKER_ENABLE
    tone(PIN_SPEAKER, 1000); // 1000 Hz tone
    #endif
    delay(100); // Beep for 100 milliseconds
    digitalWrite(PIN_LED, LOW);
    #ifdef SPEAKER_ENABLE
    noTone(PIN_SPEAKER);
    #endif
    delay(100); // Off for 100 milliseconds
  }
  delay(1000); // 1 second pause
}

void initSDCard() {
  logDebug("Initializing SD card...");

  int attempts = 0;
  while (attempts < 3 && !SD.begin(PIN_CS)) {
    attempts++;
  }
  if (attempts == 3) {
    logDebug("Failed to initialize SD card");
    while (1);
  }

  dataFile = SD.open(FILENAME, FILE_WRITE);
  dataFile.close();
}
