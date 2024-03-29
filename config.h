#define PIN_LED 17
#define PIN_SPEAKER 14
#define PIN_CS 16
#define PIN_BTN 21
#define PIN_PYRO 32
#define PIN_ADC_DT 15
#define PIN_ADC_SCK 33

#undef DEBUG
#define SPEAKER_ENABLE
#define DATA_FILENAME "/dyno.csv"
#define PROFILE_FILENAME "/profile.txt"
#define COUNTDOWN_TIME 20   // in seconds
#define TEST_DURATION 10    // in seconds
#define PYRO_DURATION 3000  // in millis
