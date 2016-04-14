#include "Teensy3DAC.h"
#include "BareSequencer.h"

BareSequencer synth;

const int LED_PIN = 13;

void setup() {
  synth.setup();

  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, true);
}

void loop() {
  synth.update(3);
}

