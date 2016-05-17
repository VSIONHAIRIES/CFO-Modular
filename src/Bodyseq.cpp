#include <stdbool.h>
#include <Arduino.h>
#include <EEPROM.h>
#include "Bodyseq.h"


// Set which pins on the Teensy3.2 board the interface elements are connected to
int Pots::potPin[] = {A0, A1};
int Keys::keyPin[] = {A2, A3, A4, A5, A6, A7, A8, A9};
int Leds::ledPin[] = {3, 4, 5, 6, 7, 8, 9, 10};
int Buttons::buttonPin[] = {11, 12, 2};

// initialise Bodyseq static variables in order for the linker to find them
int Bodyseq::mode = 0;
int Bodyseq::keys = 0;
int Bodyseq::keychange = 0;
int Bodyseq::indx = 0;
int Bodyseq::track_playing = 0;
int Bodyseq::track_selected = 0;
int Bodyseq::step_selected = 0;
int Bodyseq::note_selected = 0;
int Bodyseq::track_chained = -1;
int Bodyseq::oct = 0;
int Bodyseq::root_note = 36;
int Bodyseq::last_indx = 0;
unsigned long Bodyseq::debounce_time = 40;
unsigned long Bodyseq::debounce_time_default = 40;
unsigned long Bodyseq::debounce_time_long = 100;
int Bodyseq::octave[]{};


/////////////
// BUTTONS //
/////////////
Buttons::Buttons() {
    pinMode(buttonPin[0], INPUT_PULLUP);
    pinMode(buttonPin[1], INPUT_PULLUP);
    pinMode(buttonPin[2], INPUT_PULLUP);
    mode = &Bodyseq::mode;
    *mode = 0;
    lastMode = 0;
    buttonIndex = 0;
    buttonRead = 0;
    buttonChange = 0;
    buttonNow = 0;
    for(int i=0; i < NUM_BUTTONS; i++) {
        buttonState[i] = 0;
        buttonTime[i] = 0;
    }
    debounce_time = &Bodyseq::debounce_time;
}


void Buttons::read() {
    // buttons are active low
    for(int i = 0; i < NUM_BUTTONS; i++) {
        buttonNow = millis();
        if((buttonNow - buttonTime[i]) > *debounce_time) {
            buttonRead = digitalRead(buttonPin[i]);
            if(buttonRead != buttonState[i]) {
                buttonState[i] = buttonRead;
                buttonChange |= 1<<i;
                buttonTime[i] = buttonNow;
                // Bodyseq::state.mode = 7 - buttonState[0] - buttonState[1] * 2 - buttonState[2] * 4;
                *mode = 7 - buttonState[0] - buttonState[1] * 2 - buttonState[2] * 4;
            }
        }
    }
}




//////////
// KEYS //
//////////
Keys::Keys() {
    keys = &Bodyseq::keys;
    keyChange = &Bodyseq::keychange;
    *keyChange = 0;
    keyIndex = 0;
    keyRead = 0;
    keyValue = 0;
    keyNow = 0;
    for (int i=0; i < NUM_KEYS; i++) {
        keyState[i] = 0;
        keyTime[i] = 0;
    }
    debounce_time = &Bodyseq::debounce_time;
}


void Keys::read() {
    for (int i = 0; i < NUM_KEYS; i++) {
        keyNow = millis();
        if((keyNow - keyTime[i]) > *debounce_time) {  // TODO: debounceTime is specified in Buttons
            keyRead = analogRead(keyPin[i]);
            if(keyRead > KEY_THRESHOLD) {
                keyValue = 1;
            } else {
                keyValue = 0;
            }
            if(keyValue != keyState[i]) {
                keyState[i] = keyValue;
                *keyChange |= 1<<i;
                keyTime[i] = keyNow;
            }
            if(keyState[i]) {
                *keys |= 1<<i;
            } else {
                *keys &= ~(1<<i);
            }
        }
    }
}


//////////
// POTS //
//////////
Pots::Pots() {
    pot_hysteresis = 16;
    for(int pot=0; pot < NUM_POTS; pot++) {
        pot_hysteresis_center_value[pot] = 0;
        pot_beyond_hysteresis[pot] = 0;
        pot_lastMode[pot] = 0;
    }
    for(int mode=0; mode < NUM_MODES; mode++) {
        for(int pot=0; pot < NUM_POTS; pot++) {
            pot_callback[mode][pot] = NULL;
        }
    }
}


void Pots::attachCallback(int mode, int pot, Pot_cb callback) {
    pot_callback[mode][pot] = callback;
}



bool Pots::checkPot(int mode, int pot) {
    int read_value = analogRead(potPin[pot]);
    if(pot_lastMode[pot] == mode && pot_beyond_hysteresis[pot] == mode) {
        pot_values[mode][pot] = read_value;
        return true;
    }
    if(pot_lastMode[pot] == mode) {
        // get beyond hysteresis
        int pdiff = read_value - pot_hysteresis_center_value[pot];
        if(pdiff < 0) pdiff *= -1;
        if(pdiff > pot_hysteresis) {
            pot_values[mode][pot] = read_value;
            pot_lastMode[pot] = mode;
            pot_beyond_hysteresis[pot] = mode;
            return true;
        } else {
            return false;
        }
    } else {
        // set hysteresis point
        pot_hysteresis_center_value[pot] = read_value;
        pot_lastMode[pot] = mode;
        return false;
    }
}


void Pots::update() {
    int mode = Bodyseq::mode;
    for(int pot=0; pot < NUM_POTS; pot++) {
        if(checkPot(mode, pot)) {
            if(pot_callback[mode][pot] != NULL) {
                pot_callback[mode][pot](pot_values[mode][pot]);
            }
        }
    }
}



//////////
// LEDS //
//////////
Leds::Leds() {
    pinMode(statusLed, OUTPUT);
    for (int i = 0; i<8; i++) {
        pinMode(ledPin[i], OUTPUT);
    }
    indx = &Bodyseq::indx;
    statusLed = 13;
    chainedLedState = 0;
    ledTime = 0;
    ledPulse = 100;
}


void Leds::update() {
    ledNow = millis();
    int t = Bodyseq::track_selected;
    int s = Bodyseq::step_selected;
    int n = Bodyseq::note_selected;
    int track_playing = Bodyseq::track_playing;
    int track_chained = Bodyseq::track_chained;
    int *octave = Bodyseq::octave;
    int mode = Bodyseq::mode;
    leds = 0;

    switch(mode) {
        case 0: // PLAY TRACK
        leds |= (1 << track_playing);
        if(track_chained >= 0) {
            if((ledNow - ledTime) > ledPulse) {
                chainedLedState ^= 1;
                ledTime = ledNow;
            }
            leds |= (chainedLedState << track_chained);
        }
        break;
        case 1: // SELECT STEP
        leds = 0 | (1 << n);
        leds = leds | (octave[s + NUM_TRACKS*t] << 7);
        // for(int i=0; i<NUM_STEPS; i++) {
        //   leds |= (sample[t][s][i] << i);
        // }
        break;
        case 2: // SELECT SAMPLE
        leds |= (1 << s);
        break;
        case 3: // SELECT TRACK
        // leds |= (1 << trackSelected);
        break;
        case 4: // SELECT TRACK
        leds |= (1 << t);
        break;
        case 5: // CHAIN TRACKS PLAYING
        leds |= (1 << track_playing);
        if(track_chained >= 0) {
            if((ledNow - ledTime) > ledPulse) {
                chainedLedState ^= 1;
                ledTime = ledNow;
            }
            leds |= (chainedLedState << track_chained);
        }
        break;
        case 6: // COPY TRACK
        leds |= (1 << t);
        break;
        case 7: // CLEAR TRACK
        // for(int i=0; i<NUM_KEYS; i++) {
        //   for(int j=0; j<NUM_SAMPLES; j++) {
        //     for(int k=0; k<NUM_STEPS; k++) {
        //       leds |= (sample[i][j][k] << i);
        //     }
        //   }
        // }
        break;
        default:
        break;
    }
    // indx = seq.getPosition(t);
    for (int i = 0; i<8; i++) {
        // leds ^= (1 << indx);
        leds |= (1 << *indx);
        digitalWrite(ledPin[i], leds & (1 << i));
    }
}


void Leds::startupAnimation() {
    digitalWrite(statusLed, HIGH);
    for (int i = 0; i<8; i++) {
        digitalWrite(ledPin[i],HIGH);
        delay(30);
    }
    for (int i = 0; i<8; i++) {
        digitalWrite(ledPin[i],LOW);
        delay(30);
    }

    for (int i = 0; i<8; i++) {
        digitalWrite(ledPin[7-i],HIGH);
        delay(30);
    }
    for (int i = 0; i<8; i++) {
        digitalWrite(ledPin[7-i],LOW);
        delay(30);
    }
    digitalWrite(statusLed, LOW);
    delay(100);
}




///////////////////////
// BODYSEQ INTERFACE //
///////////////////////
Bodyseq::Bodyseq() {
    track_playing = 0;
    track_selected = 0;
    step_selected = 0;
    note_selected = 0;
    track_chained = -1;
    oct = 0;
    root_note = 36;
    last_indx = 0;
    debounce_time_default = 40;
    debounce_time_long = 100;
    debounce_time = debounce_time_default;
}


void Bodyseq::attachSequencer(Sequencer *s) {
    sequencer = s;
}


void Bodyseq::init() {
    setupSequences();
    leds.startupAnimation();
}


void Bodyseq::updatePosition() {
    indx = sequencer->getPosition(track_selected);
    if(indx != last_indx) {
        // Serial.print("indx : ");
        // Serial.println(indx);
        if(indx >= NUM_STEPS-1) {
            if(track_chained < 0);
            else {
                int t = track_playing;
                track_playing = track_chained;
                track_chained = t;
                // Serial.print("trackPlaying : ");
                // Serial.print(trackPlaying);
                // Serial.print(" | trackChained : ");
                // Serial.println(trackChained);
                sequencer->setInternal(track[track_chained], false);
                sequencer->setInternal(track[track_playing], true);
            }
        }
        last_indx = indx;
    }
}


void Bodyseq::update() {
    buttons.read();
    keypads.read();
    updatePosition();
    pots.update();

    switch(mode) {
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
    leds.update();
}


void Bodyseq::playTrack() {
    if(keychange && keys) {
        // Serial.print("PLAY TRACK ");
        for(int i = 0; i < NUM_KEYS; i++) {
            sequencer->setInternal(track[i], false);
            if(keys & (1 << i)) {
                if(track_playing == i) track_chained = -1;
                if(track_chained < 0) track_playing = i;
                else track_chained = i;
                // Serial.println(i);
            }
        }
        keychange = 0;
    }
    sequencer->setInternal(track[track_playing], true);
}


void Bodyseq::selectNote() {
    note_selected = notes[step_selected + NUM_TRACKS * track_selected];
    oct = octave[step_selected + NUM_TRACKS * track_selected];
    if(keychange) {
        for(int i = 0; i < NUM_KEYS-1; i++) {
            if(keys & (1 << i)) {
                if(note_selected == i) note_selected = 255;
                else note_selected = i;
            }
            notes[step_selected + NUM_TRACKS * track_selected] = note_selected;
            // Serial.print("noteSelected: ");
            // Serial.println(notes[stepSelected + NUM_TRACKS * trackSelected]);
            EEPROM.write(step_selected + track_selected * NUM_TRACKS, note_selected);
        }
        if(keys & (1 << 7)) {
            oct ^= 1;
            octave[step_selected + NUM_TRACKS * track_selected] = oct;
            EEPROM.write(step_selected + track_selected * NUM_TRACKS + OCT_EEPROM_OFFSET, oct);
        }
        if(note_selected == 255) {
            noteValues[0] = 0;
        } else {
            noteValues[0] = root_note + scale[notes[step_selected + NUM_TRACKS * track_selected]]
            + octave[step_selected + NUM_TRACKS * track_selected] * 12;
        }
        sequencer->insertNotes(track[track_selected], noteValues, 1, step_selected);
        keychange = 0;
    }
}

void Bodyseq::selectStep() {
    if(keychange) {
        // Serial.println("SELECT STEP");
        for(int i = 0; i < NUM_KEYS; i++) {
            // code here
            if(keys & (1 << i)) {
                step_selected = i;
                // Serial.print("Step selected: ");
                // Serial.println(stepSelected);
            }
        }
        keychange = 0;
    }
}

void Bodyseq::selectTrack() {
    if(keychange) {
        // Serial.println("SELECT TRACK");
        for(int i = 0; i < NUM_KEYS; i++) {
            if(keys & (1 << i)) {
                track_selected = i;
            }
        }
        keychange = 0;
    }
}


void Bodyseq::chainTrack() {
    if(keychange) {
        // Serial.println("SELECT TRACK");
        for(int i = 0; i < NUM_KEYS; i++) {
            if(keys & (1 << i)) {
                if(i == track_chained) track_chained = -1;
                else track_chained = i;
                // Serial.print("trackPlaying : ");
                // Serial.print(trackPlaying);
                // Serial.print(" | trackChained : ");
                // Serial.println(trackPlaying);
            }
        }
        keychange = 0;
    }
}


void Bodyseq::copyTrack() {
    if(keychange) {
        debounce_time = debounce_time_long;
        // Serial.println("COPY TRACK");
        for(int i = 0; i < NUM_KEYS; i++) {
            if(keys & (1 << i)) {
                for(int j=0; j<NUM_STEPS; j++) {
                    notes[8*i + j] = notes[8*track_selected + j];
                    octave[8*i + j] = octave[8*track_selected + j];
                    noteValues[j] = root_note + scale[notes[8*track_selected + j]]
                    + octave[8*track_selected + j] * 12;
                    EEPROM.write(j + i * NUM_TRACKS, notes[8*i + j]);
                    EEPROM.write(j + i * NUM_TRACKS + OCT_EEPROM_OFFSET, octave[8*i + j]);
                }
                track_selected = i;
                sequencer->insertNotes(track[track_selected], noteValues, 8, step_selected);
            }
        }
        keychange = 0;
        debounce_time = debounce_time_default;
    }
}


void Bodyseq::clearTrack() {
    if(keychange) {
        // Serial.println("CLEAR TRACK");
        for(int i = 0; i < NUM_KEYS; i++) {
            if(keys & (1 << i)) {
                for(int j=0; j<NUM_STEPS; j++) {
                    notes[8*i + j] = 0;
                    octave[8*i + j] = 0;
                    noteValues[0] = 0;
                    sequencer->insertNotes(track[track_selected], noteValues, 1, step_selected);
                    EEPROM.write(j + i * NUM_TRACKS, 0);
                    EEPROM.write(j + i * NUM_TRACKS + OCT_EEPROM_OFFSET, 0);
                }
            }
        }
        keychange = 0;
    }
}


void Bodyseq::setupSequences() {
    for(int i = 0; i < NUM_TRACKS; i++) {
        for(int j = 0; j < NUM_STEPS; j++) {
            notes[NUM_TRACKS*i + j] = EEPROM.read(i * NUM_TRACKS + j);
            octave[NUM_TRACKS*i + j] = EEPROM.read(i * NUM_TRACKS + j + OCT_EEPROM_OFFSET);
            if(octave[NUM_TRACKS*i + j] > 1) octave[NUM_TRACKS*i + j] = 1;
            // notes[8*i + j] = j;
        }
        track[i] = sequencer->newSequence(NOTE_16, 8, LOOP);
        // Serial.print("Track created for sequence ");
        // Serial.print(track[i]);
        // Serial.print(" on position ");
        // Serial.println(i);
        // seq.startSequence(track[i]);
        for(int j = 0; j < NUM_STEPS; j++) {
            int note = notes[8 * i + j];
            if(note == 255) {
                noteValues[j] = 0;
                // Serial.println("ENCOUNTERED -1");
            } else {
                noteValues[j] = root_note + scale[notes[8 * i + j]] + octave[8 * i + j] * 12;
            }
            // noteValues[j] = rootNote + scale[notes[8 * i + j]] + octave[8 * i + j] * 12;
            // noteValues[j] = rootNote;
        }
        sequencer->insertNotes(track[i], noteValues, 8, 0);
        sequencer->setInternal(track[i], false);
        sequencer->setExternal(track[i], false);
        // Serial.print("Internal set to ");
        // Serial.println(seq.getInternal(track[i]));
    }
    for(int i = 0; i < NUM_TRACKS; i++) {
        // Serial.print("Rechecking sequence on position ");
        // Serial.print(i);
        // Serial.print(" - set to sequence index ");
        // Serial.print(track[i]);
        // Serial.print(" - Internal = ");
        // Serial.println(seq.getInternal(track[i]));
    }
}


void Memory::wipe_eeprom() {
    for(int i = 0; i < NUM_TRACKS; i++) {
        for(int j = 0; j < NUM_STEPS; j++) {
            EEPROM.write(j + i * NUM_TRACKS, 0);
            EEPROM.write(j + i * NUM_TRACKS + OCT_EEPROM_OFFSET, 0);
        }
    }
}
