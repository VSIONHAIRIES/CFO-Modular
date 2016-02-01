#define MIDI_CHANNEL 1
#include <math.h>
#include <EEPROM.h>
#include <SimpleSequencer.h>

#define NUM_TRACKS 8
#define NUM_STEPS 8
// #define NUM_NOTES 8
#define NUM_LEDS 8

// int track[8];

// int _bpm;

int trackPlaying = 0;
int trackSelected = 0;
int stepSelected = 0;
int noteSelected = 0;
int trackChained = -1;

// int notes[64];
// int octave[64];
// int noteValues[8];

int8_t notes[NUM_TRACKS][NUM_STEPS];
int8_t octaves[NUM_TRACKS][NUM_STEPS];
int note;
int oct = 0;

int s1;
int indx = 0;

bool inStartupMode = true;

const int scale[] = {0, 2, 3, 5, 7, 8, 10, 12};
//const int octave[] = {-24, -12, 0, 12, 24};
int rootNote = 36;

//int var = 0;
//const int pot1 = A0, pot2 = A1;


//////////
// LEDS //
//////////

const int seqLed[] = {3,4,5,6,7,8,9,10};
const int statusLed1 = 13;
int indxLED;
int leds;
int chainedLedState = 0;
unsigned long ledNow;
unsigned long ledTime = 0;
unsigned long ledPulse = 100;

//boolean debug = true;


/////////////
// BUTTONS //
/////////////
#define NUM_BUTTONS 3
#define NUM_MACHINE_STATES (1 << NUM_BUTTONS)
int mode;
// const int debounceTime = 40;
unsigned long debounceTime = 100;
unsigned long debounceTimeDefault = 100;
unsigned long debounceTimeLong = 500;
const int buttonPin [] = {11,12,2};
int buttonIndex = 0;
int buttonRead = 0;
int buttonChange = 0;
int buttonState[] = {0, 0, 0};
unsigned long buttonNow = 0;
unsigned long buttonTime[] = {0, 0, 0};
int machineState = 0;
int lastMachineState = 0;

//////////
// KEYS //
//////////
#define NUM_KEYS 8
#define KEY_THRESHOLD 24
const int keyPin[] = {A2,A3,A4,A5,A6,A7,A8,A9};
int keyIndex = 0;
int keyRead = 0;
int keyValue = 0;
int keyChange = 0;
int keyState[] = {0, 0, 0, 0, 0, 0, 0, 0};
unsigned long keyNow = 0;
unsigned long keyTime[] = {0, 0, 0, 0, 0, 0, 0, 0};
int keys;

////////////////////
// POTENTIOMETERS //
////////////////////

#define NUM_POTS 2
const int pots[NUM_POTS] = {A0, A1};
const int pot_hysteresis = 16;
int pot_values[NUM_POTS][NUM_MACHINE_STATES];
int pot_hysteresis_center_value[NUM_POTS] = {0, 0};
int pot_beyond_hysteresis[NUM_POTS] = {0, 0};
int pot_lastMachineState[NUM_POTS] = {0, 0};


// Synthesizer variables
SimpleSequencer synth;
int cutoffModAmount = 0;
int cutoff = 0;

void setup() {

  usbMIDI.setHandleNoteOff(OnNoteOff);
  usbMIDI.setHandleNoteOn(OnNoteOn);
  usbMIDI.setHandleControlChange(OnControlChange);
  usbMIDI.setHandleRealTimeSystem(RealTimeSystem);

  delay(2000);

  synth.start();
  fltr.cutoffIn_ptr = &cutoff;
	fltr.cutoffModAmountIn_ptr = &cutoffModAmount;

  seq.init(120);
  seq.setInternalClock(true);
  s1 = seq.newSequence(NOTE_16, &s1cb);
//  resetMemory();
  // setupSequences();
  loadSequences();
  initInterface();
  Sequencer.startSequence(s1);

}


void loop() {
  seq.update();
  usbMIDI.read();
  // midi.checkSerialMidi();
  readButtons();
  readKeys();
  // checkBPM();
  updatePots();
  switch(machineState) {
    case 0:
      playTrack();
      break;
    case 1:
      selectNote();
      break;
    case 2:
      selectStep();
      break;
    case 3: // nothing
      break;
    case 4:
      selectTrack();
      break;
    case 5: // nothing
      break;
    case 6: 
      copyTrack();
      break;
    case 7:
      clearTrack();
      break;
    default:
      break;
  }
  updateLEDs();
}


void playTrack() {
  if(keyChange && keys) {
    Serial.print("PLAY TRACK ");
    for(int i = 0; i < NUM_KEYS; i++) {
      seq.setInternal(track[i], false);
      if(keys & (1 << i)) {
        if(trackPlaying == i) trackChained = -1;
        if(trackChained < 0) trackPlaying = i;
        else trackChained = i;
        Serial.println(i);
      }
    }
    keyChange = 0;
  }
  seq.setInternal(track[trackPlaying], true);
}


void selectNote() {
  noteSelected = notes[stepSelected + 8 * trackSelected];
  oct = octave[stepSelected + 8 * trackSelected];
  if(keyChange) {
    for(int i = 0; i < NUM_KEYS-1; i++) {
      if(keys & (1 << i)) noteSelected = i;
      notes[stepSelected + 8 * trackSelected] = noteSelected;
    }
    if(keys & (1 << 7)) {
      oct ^= 1;
      octave[stepSelected + 8 * trackSelected] = oct;
    }
    for(int i = 0; i < NUM_STEPS; i++) {
      noteValues[i] = rootNote + scale[notes[8 * trackSelected + i]] + octave[i + 8 * trackSelected] * 12;
      seq.insertNotes(track[trackSelected], noteValues, 8, 0);
    }
    keyChange = 0;
  }
}


void selectStep() {
  if(keyChange) {
    Serial.println("SELECT STEP");
    for(int k = 0; k < NUM_KEYS; k++) {
      if(keys & (1 << k)) {
        stepSelected = k;
        // int j = sampleSelected;
        // int i = trackSelected;
        // sample[i][j][k] ^= 1;
        // EEPROM.write(k + NUM_SAMPLES * (j + i * NUM_TRACKS), sample[i][j][k]);
      }
    }
    keyChange = 0;
  }
}


// void selectStep() {
//   if(keyChange) {
//     Serial.println("SELECT STEP");
//     for(int i = 0; i < NUM_KEYS; i++) {
//   // code here
//       if(keys & (1 << i)) {
//         stepSelected = i;
//         Serial.print("Step selected: ");
//         Serial.println(stepSelected);
//       }
//     }
//     keyChange = 0;
//   }
// }

void selectTrack() {
  if(keyChange) {
    Serial.println("SELECT TRACK");
    for(int k = 0; k < NUM_KEYS; k++) {
      if(keys & (1 << k)) {
        trackSelected = k;
      }
    }
    keyChange = 0;
  }
}


void chainTrack() {
  if(keyChange) {
    Serial.println("SELECT TRACK");
    for(int k = 0; k < NUM_KEYS; k++) {
      if(keys & (1 << k)) {
        if(k == trackChained) trackChained = -1;
        else trackChained = k;
      }
    }
    keyChange = 0;
  }
}


void copyTrack() {
  if(keyChange) {
    debounceTime = debounceTimeLong;
    Serial.println("COPY TRACK");
    for(int i = 0; i < NUM_KEYS; i++) {
      if(keys & (1 << i)) {
        for(int j=0; j<NUM_STEPS; j++) {
          sample[i][j] = sample[trackSelected][j];
          EEPROM.write(j + i * NUM_TRACKS, sample[i][j]);
        }
      }
      trackSelected = i;
    }
    keyChange = 0;
    debounceTime = debounceTimeDefault;
  }
}


void clearTrack() {
  if(keyChange) {
    Serial.println("CLEAR TRACK");
    for(int i = 0; i < NUM_KEYS; i++) {
      if(keys & (1 << i)) {
        for(int j=0; j<NUM_STEPS; j++) {
          sample[i][j] = 0;
          EEPROM.write(j + i * NUM_TRACKS, 0);
        }
      }
    }
    keyChange = 0;
  }
}


void resetMemory() {
  for(int i=0; i<NUM_TRACKS; i++) {
    for(int j=0; j<NUM_STEPS; j++) {
      EEPROM.write(j + i * NUM_TRACKS, 0);
    }
  }
}


void loadSequences() {
  for(int i=0; i<NUM_TRACKS; i++) {
    for(int j=0; j<NUM_STEPS; j++) {
      notes[i][j] = EEPROM.read(j + i * NUM_TRACKS);
    }
  }
}

void s1cb() {
  indxLED = indx;
  for(int i=0; i<NUM_STEPS; i++) {
    if(sample[trackPlaying][i][indx]) Music.noteOnSample(i);
  }
  indx++;
  if(indx >= NUM_STEPS) {
    indx = 0;
    if(trackChained < 0);
    else {
      int t = trackPlaying;
      trackPlaying = trackChained;
      trackChained = t;
    }
  }
}



void setupSequences() {
  for(int i = 0; i < NUM_TRACKS; i++) {
    for(int j = 0; j < NUM_STEPS; j++) {

      // notes[8*i + j] = j;
    }
    track[i] = seq.newSequence(NOTE_16, 8, LOOP);
    Serial.print("Track created for sequence ");
    Serial.print(track[i]);
    Serial.print(" on position ");
    Serial.println(i);
    seq.startSequence(track[i]);
    for(int j = 0; j < NUM_STEPS; j++) {
      // noteValues[j] = rootNote + scale[notes[8 * i + j]] + octave[8 * i + j] * 12;
      noteValues[j] = rootNote;
    }
    seq.insertNotes(track[i], noteValues, 8, 0);
    seq.setInternal(track[i], false);
    seq.setExternal(track[i], false);
    Serial.print("Internal set to ");
    Serial.println(seq.getInternal(track[i]));
  }
  for(int i = 0; i < NUM_TRACKS; i++) {
    Serial.print("Rechecking sequence on position ");
    Serial.print(i);
    Serial.print(" - set to sequence index ");
    Serial.print(track[i]);
    Serial.print(" - Internal = ");
    Serial.println(seq.getInternal(track[i]));
  }
}
