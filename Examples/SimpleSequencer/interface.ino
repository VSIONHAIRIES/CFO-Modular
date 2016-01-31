/////////////
// BUTTONS //
/////////////
void readButtons() {
  // buttons are active low
  for(int i = 0; i < NUM_BUTTONS; i++) {
//    int i = buttonIndex++;
//    if(buttonIndex >= NUM_BUTTONS) buttonIndex = 0;
    buttonNow = millis();
    if((buttonNow - buttonTime[i]) > debounceTime) {
      buttonRead = digitalRead(buttonPin[i]);
      if(buttonRead != buttonState[i]) {
        buttonState[i] = buttonRead;
        buttonChange |= 1<<i;
        buttonTime[i] = buttonNow;
        machineState = 7 - buttonState[0] - buttonState[1] * 2 - buttonState[2] * 4;
      }
    }
  }
  // Serial.println("Read Buttons");
}


//////////
// KEYS //
//////////
void readKeys() {
  for (int i = 0; i < NUM_KEYS; i++) {
//    int i = keyIndex++;
//    if(keyIndex >= NUM_KEYS) keyIndex = 0;
    keyNow = millis();
    if((keyNow - keyTime[i]) > debounceTime) {
      keyRead = analogRead(keyPin[i]);
//      if(i==4) Serial.println(keyRead);
      if(keyRead > KEY_THRESHOLD) {
        keyValue = 1;
      } else {
        keyValue = 0;
      }
//      keyValue = (keyRead > KEY_THRESHOLD) ? 1 : 0 ;
      if(keyValue != keyState[i]) {
        keyState[i] = keyValue;
        keyChange |= 1<<i;
        keyTime[i] = keyNow;
      }
      if(keyState[i]) {
        keys |= 1<<i;
      } else {
        keys &= ~(1<<i);
      }
    }
  }
}


//////////
// POTS //
//////////

bool checkPot(int p, int ms) {
  int read_value = analogRead(pots[p]);
  if(pot_lastMachineState[p] == ms && pot_beyond_hysteresis[p] == ms) {
    pot_values[p][ms] = read_value;
    return true;
  }
  if(pot_lastMachineState[p] == ms) {
    // get beyond hysteresis
    int pdiff = read_value - pot_hysteresis_center_value[p];
    if(pdiff < 0) pdiff *= -1;
    if(pdiff > pot_hysteresis) {
      pot_values[p][ms] = read_value;
      pot_lastMachineState[p] = ms;
      pot_beyond_hysteresis[p] = ms;
      return true;
    } else {
      return false;
    }
  } else {
    // set hysteresis point
    pot_hysteresis_center_value[p] = read_value;
    pot_lastMachineState[p] = ms;
    return false;
  }
}


void updatePots() {
  switch(machineState) {
    case 0:                 // NO BUTTONS
      if(checkPot(0, machineState)) cutoff = pot_values[0][machineState] << 21;
      if(checkPot(1, machineState)) cutoffModAmount = pot_values[1][machineState] << 21;
      break;
    case 1:                 // BUTTON 1
      if(checkPot(0, machineState)) env2.setAttack(pot_values[0][machineState] >> 3);
      if(checkPot(1, machineState)) {
        env2.setDecay(pot_values[1][machineState] >> 3);
        env2.setRelease(pot_values[1][machineState] >> 3);
      }
      break;
    case 2:                 // BUTTON 2
      if(checkPot(0, machineState)) wave2.setSemitone((pot_values[0][machineState] >> 4) - 32);
      if(checkPot(1, machineState)) wave2.setDetune(pot_values[1][machineState] >> 2);
      break;
    case 3:                 // BUTTON 1 + 2
      if(checkPot(0, machineState));
      if(checkPot(1, machineState));
      break;
    case 4:                 // BUTTON 3
      if(checkPot(0, machineState)) wave1.setWaveform(pot_values[0][machineState] >> 6);
      if(checkPot(1, machineState)) wave2.setWaveform(pot_values[1][machineState] >> 6);
      break;
    case 5:                 // BUTTON 1 + 3
      if(checkPot(0, machineState)) setBPM(pot_values[0][machineState] >> 2);
      if(checkPot(1, machineState));
      break;
    case 6:                 // BUTTON 2 + 3
      if(checkPot(0, machineState));
      if(checkPot(1, machineState));
      break;
    case 7:                 // BUTTON 1 + 2 + 3
      if(checkPot(0, machineState));
      if(checkPot(1, machineState));
      break;
    default:
      break;
  }
}


void setBPM(uint8_t bpm) {
// int bpm = analogRead(A0)>>2;
// if(bpm != _bpm) {
//   _bpm = bpm;
  // Serial.print("BPM set to ");
  // Serial.println(_bpm);
  seq.setbpm(bpm);
  if(bpm == 0) {
    midi.setMidiIn(true);
    midi.setMidiThru(true);
    midi.setMidiOut(true);
    midi.setMidiClockIn(true);
    midi.setMidiClockThru(true);
    midi.setMidiClockOut(true);
    seq.setInternalClock(false);
  } else {
    midi.setMidiIn(false);
    midi.setMidiThru(false);
    midi.setMidiOut(false);
    midi.setMidiClockIn(false);
    midi.setMidiClockThru(false);
    midi.setMidiClockOut(false);
    seq.setInternalClock(true);
//      Sequencer.sequencerContinue();
  }
}


/////////////////////
// OTHER FUNCTIONS //
/////////////////////

void initInterface() {
  pinMode(buttonPin[0], INPUT_PULLUP);
  pinMode(buttonPin[1], INPUT_PULLUP);
  pinMode(buttonPin[2], INPUT_PULLUP);
  pinMode(statusLed1, OUTPUT);
  for (int i = 0; i<8; i++) {
    pinMode(seqLed[i], OUTPUT);
  }
  startupAnimation();
}


void updateLEDs() {
    switch(machineState) {
      case 0:
        leds = 0 | (1 << trackSelected);
        break;
      case 1:
        leds = 0 | (1 << noteSelected);
        leds = leds | (octave[stepSelected + 8 * trackSelected] << 7);
        break;
      case 2:
        leds = 0 | (1 << stepSelected);
        break;
      case 3: // nothing
        break;
      case 4:
        leds = 0 | (1 << trackSelected);
        break;
      case 5: // nothing
        break;
      case 6: // nothing
        break;
      case 7: // nothing
        break;
      default:
        break;
    }
  for (int i = 0; i<8; i++) {
    digitalWrite(seqLed[i], leds & (1 << i));
  }
}


void startupAnimation() {
  digitalWrite(statusLed1, HIGH);

  for (int i = 0; i<8; i++) {
    digitalWrite(seqLed[i],HIGH);
    delay(30);
  }
  for (int i = 0; i<8; i++) {
    digitalWrite(seqLed[i],LOW);
    delay(30);
  }

  for (int i = 0; i<8; i++) {
    digitalWrite(seqLed[7-i],HIGH);
    delay(30);
  }
  for (int i = 0; i<8; i++) {
    digitalWrite(seqLed[7-i],LOW);
    delay(30);
  }

  digitalWrite(statusLed1, LOW);

  delay(100);
}
