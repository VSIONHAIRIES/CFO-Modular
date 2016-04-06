#include <math.h>
#include <EEPROM.h>
#include <SimpleSequencer.h>

#define MIDI_CHANNEL 1
#define NUM_TRACKS 8
#define NUM_STEPS 8
#define OCT_EEPROM_OFFSET (NUM_TRACKS * NUM_STEPS)
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

int trackPlaying = 0;
int trackSelected = 0;
int stepSelected = 0;
int noteSelected = 0;
int trackChained = -1;
int oct = 0;

int notes[64];
int octave[64];
int noteValues[8];
int note; // is this needed?

//int var = 0;
//const int pot1 = A0, pot2 = A1;

//boolean debug = true;

//////////
// LEDS //
//////////

const int seqLed[] = {3,4,5,6,7,8,9,10};
const int statusLed1 = 13;
int indx;
int last_indx = 0;
int leds;
int chainedLedState = 0;
unsigned long ledNow;
unsigned long ledTime = 0;
unsigned long ledPulse = 100;


/////////////
// BUTTONS //
/////////////
#define NUM_BUTTONS 3
#define NUM_MACHINE_STATES ((int)(pow(2, NUM_BUTTONS)))
unsigned long debounceTime = 40;
unsigned long debounceTimeDefault = 40;
unsigned long debounceTimeLong = 100;
// const int debounceTime = 40;
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


void setup() {

  synth.start();
  fltr.cutoffIn_ptr = &cutoff;
	fltr.cutoffModAmountIn_ptr = &cutoffModAmount;

  wave1.setWaveform(SAW);
  wave2.setWaveform(SAW);

  // delay(2000);
  seq.init(120);
  seq.setInternalClock(true);
  setupSequences();
  setBPM(0);
  initInterface();
}


void loop() {
  seq.update();
  usbMIDI.read();
  midi.checkSerialMidi();
  readButtons();
  readKeys();
  updatePosition();
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
      chainTrack();
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


// void playTrack() {
//   if(keyChange) {
//     Serial.println("PLAY TRACK");
//   // code here
//   keyChange = 0;
//   }
// }

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
  noteSelected = notes[stepSelected + NUM_TRACKS * trackSelected];
  oct = octave[stepSelected + NUM_TRACKS * trackSelected];
  if(keyChange) {
    for(int i = 0; i < NUM_KEYS-1; i++) {
      if(keys & (1 << i)) {
        if(noteSelected == i) noteSelected = 255;
        else noteSelected = i;
      }
      notes[stepSelected + NUM_TRACKS * trackSelected] = noteSelected;
      // Serial.print("noteSelected: ");
      // Serial.println(notes[stepSelected + NUM_TRACKS * trackSelected]);
      EEPROM.write(stepSelected + trackSelected * NUM_TRACKS, noteSelected);
    }
    if(keys & (1 << 7)) {
      oct ^= 1;
      octave[stepSelected + NUM_TRACKS * trackSelected] = oct;
      EEPROM.write(stepSelected + trackSelected * NUM_TRACKS + OCT_EEPROM_OFFSET, oct);
    }
    // for(int i = 0; i < NUM_STEPS; i++) {
    //   noteValues[i] = rootNote + scale[notes[8 * trackSelected + i]] + octave[i + 8 * trackSelected] * 12;
    //   seq.insertNotes(track[trackSelected], noteValues, 8, 0);
    // }
    // int note_position = scale[notes[stepSelected + NUM_TRACKS * trackSelected]];
    // int note_octave = octave[stepSelected + NUM_TRACKS * trackSelected] * 12;
    // noteValues[0] = rootNote + note_position + note_octave;
    if(noteSelected == 255) {
      noteValues[0] = 0;
    } else {
      noteValues[0] = rootNote + scale[notes[stepSelected + NUM_TRACKS * trackSelected]]
                               + octave[stepSelected + NUM_TRACKS * trackSelected] * 12;
    }
    // Serial.print("Inserting note in sequencer: ");
    // Serial.println(noteValues[0]);
    // Serial.print("rootNote: ");
    // Serial.println(rootNote);
    // Serial.print("note_position: ");
    // Serial.println(note_position);
    // Serial.print("note_octave: ");
    // Serial.println(note_octave);
    seq.insertNotes(track[trackSelected], noteValues, 1, stepSelected);
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
  if(keyChange) {
    Serial.println("SELECT TRACK");
    for(int i = 0; i < NUM_KEYS; i++) {
      if(keys & (1 << i)) {
        trackSelected = i;
      }
    }
    keyChange = 0;
  }
}


void chainTrack() {
  if(keyChange) {
    Serial.println("SELECT TRACK");
    for(int i = 0; i < NUM_KEYS; i++) {
      if(keys & (1 << i)) {
        if(i == trackChained) trackChained = -1;
        else trackChained = i;
        Serial.print("trackPlaying : ");
        Serial.print(trackPlaying);
        Serial.print(" | trackChained : ");
        Serial.println(trackPlaying);
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
          notes[8*i + j] = notes[8*trackSelected + j];
          octave[8*i + j] = octave[8*trackSelected + j];
          noteValues[j] = rootNote + scale[notes[8*trackSelected + j]]
                                   + octave[8*trackSelected + j] * 12;
          EEPROM.write(j + i * NUM_TRACKS, notes[8*i + j]);
          EEPROM.write(j + i * NUM_TRACKS + OCT_EEPROM_OFFSET, octave[8*i + j]);
        }
        trackSelected = i;
        seq.insertNotes(track[trackSelected], noteValues, 8, stepSelected);
      }
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
          notes[8*i + j] = 0;
          octave[8*i + j] = 0;
          noteValues[0] = 0;
          seq.insertNotes(track[trackSelected], noteValues, 1, stepSelected);
          EEPROM.write(j + i * NUM_TRACKS, 0);
          EEPROM.write(j + i * NUM_TRACKS + OCT_EEPROM_OFFSET, 0);
        }
      }
    }
    keyChange = 0;
  }
}


void setupSequences() {
  for(int i = 0; i < NUM_TRACKS; i++) {
    for(int j = 0; j < NUM_STEPS; j++) {
      notes[NUM_TRACKS*i + j] = EEPROM.read(i * NUM_TRACKS + j);
      octave[NUM_TRACKS*i + j] = EEPROM.read(i * NUM_TRACKS + j + OCT_EEPROM_OFFSET);
      if(octave[NUM_TRACKS*i + j] > 1) octave[NUM_TRACKS*i + j] = 1;
      // notes[8*i + j] = j;
    }
    track[i] = seq.newSequence(NOTE_16, 8, LOOP);
    Serial.print("Track created for sequence ");
    Serial.print(track[i]);
    Serial.print(" on position ");
    Serial.println(i);
    // seq.startSequence(track[i]);
    for(int j = 0; j < NUM_STEPS; j++) {
      int note = notes[8 * i + j];
      if(note == 255) {
        noteValues[j] = 0;
        Serial.println("ENCOUNTERED -1");
      }
      else {
        noteValues[j] = rootNote + scale[notes[8 * i + j]] + octave[8 * i + j] * 12;
      }
      // noteValues[j] = rootNote + scale[notes[8 * i + j]] + octave[8 * i + j] * 12;
      // noteValues[j] = rootNote;
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
