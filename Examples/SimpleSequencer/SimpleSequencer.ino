#include <SimpleSequencer.h>

#define MIDI_CHANNEL 1
#define NUM_TRACKS 8
#define NUM_STEPS 8
#define NUM_LEDS 8

SimpleSequencer synth;

int cutoffModAmount = 0;
int cutoff = 0;

int mode;

int track[8];

int _bpm;

const int scale[] = {0, 2, 3, 5, 7, 8, 10, 12};
//const int octave[] = {-24, -12, 0, 12, 24};
int rootNote = 36;

int trackPlaying = 0 ;
int trackSelected = 0;
int stepSelected = 0;
int noteSelected = 0;
int oct = 0;

int notes[64];
int octave[64];
int noteValues[8];

int leds;
int note;

//int var = 0;
//const int pot1 = A0, pot2 = A1;

// old stuff
const int seqLed[] = {3,4,5,6,7,8,9,10};
const int statusLed1 = 13;
//boolean debug = true;


/////////////
// BUTTONS //
/////////////
#define NUM_BUTTONS 3
const int debounceTime = 40;
const int buttonPin [] = {11,12,2};
int buttonIndex = 0;
int buttonRead = 0;
int buttonChange = 0;
int buttonState[] = {0, 0, 0};
unsigned long buttonNow = 0;
unsigned long buttonTime[] = {0, 0, 0};
int machineState = 0;

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


void setup() {

  usbMIDI.setHandleNoteOff(OnNoteOff);
  usbMIDI.setHandleNoteOn(OnNoteOn);
  usbMIDI.setHandleControlChange(OnControlChange);
  usbMIDI.setHandleRealTimeSystem(RealTimeSystem);

  synth.start();
  fltr.cutoffIn_ptr = &cutoff;
	fltr.cutoffModAmountIn_ptr = &cutoffModAmount;

  delay(2000);
  seq.init(120);
  seq.setInternalClock(true);
  setupSequences();
  initInterface();
}


void loop() {
  seq.update();
  usbMIDI.read();
  // midi.checkSerialMidi();
  readButtons();
  readKeys();
  checkBPM();
  // cutoff = analogRead(A0) << 21;
  cutoff = 32 << 24;
	cutoffModAmount = analogRead(A1) << 21;

  //  if(buttonChange || keyChange) {
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
      case 6: // nothing
        break;
      case 7: // nothing
        break;
      default:
        break;
    }
    updateLEDs();
//    buttonChange = 0;
//  }
}


void playTrack() {
  if(keyChange) {
    Serial.println("PLAY TRACK");
  // code here
  keyChange = 0;
  }
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
    for(int i = 0; i < NUM_KEYS; i++) {
  // code here
      if(keys & (1 << i)) {
        stepSelected = i;
        Serial.print("Step selected: ");
        Serial.println(stepSelected);
      }
    }
    keyChange = 0;
  }
}

void selectTrack() {
//  if(keys > 0) {
  if(keyChange) {
    Serial.println("SELECT TRACK");
    for(int i = 0; i < NUM_KEYS; i++) {
//      Serial.print("Rechecking sequence on position ");
//      Serial.print(i);
//      Serial.print(". It is set to sequence index ");
//      Serial.println(track[i]);
      seq.setInternal(track[i], false);
      if(keys & (1 << i)) {
        trackSelected = i;
        Serial.print("trackSelected shows ");
        Serial.print(trackSelected);
        Serial.print(" - Track selected: ");
        Serial.println(track[trackSelected]);
      }
      seq.setInternal(track[trackSelected], true);
    }
    keyChange = 0;
  }
  // code here
}


void setupSequences() {
  for(int i = 0; i < NUM_TRACKS; i++) {
    for(int j = 0; j < NUM_STEPS; j++) {
      notes[8*i + j] = j;
    }
    track[i] = seq.newSequence(NOTE_16, 8, LOOP);
    Serial.print("Track created for sequence ");
    Serial.print(track[i]);
    Serial.print(" on position ");
    Serial.println(i);
    seq.startSequence(track[i]);
    for(int j = 0; j < NUM_STEPS; j++) {
      noteValues[j] = rootNote + scale[notes[8 * i + j]] + octave[8 * i + j] * 12;
    }
    seq.insertNotes(track[i], noteValues, 8, 0);
    seq.setInternal(track[i], true);
    seq.setExternal(track[i], false);
    Serial.print("Internal set to ");
    Serial.println(seq.getInternal(track[i]));
  }
  for(int i = 0; i < NUM_TRACKS; i++) {
    Serial.print("Rechecking sequence on position ");
    Serial.print(i);
    Serial.print(" - set to sequence index ");
    Serial.println(track[i]);
  }
}
