#ifndef BODYSEQ_H
#define BODYSEQ_H

#include <Arduino.h>
#include <stdint.h>
#include "Sequencer.h"

#define NUM_BUTTONS 3
#define NUM_MODES (1 << NUM_BUTTONS)
#define NUM_KEYS 8
#define KEY_THRESHOLD 24
#define NUM_POTS 2
#define NUM_LEDS 8
#define NUM_TRACKS 8
#define NUM_STEPS 8
#define OCT_EEPROM_OFFSET (NUM_TRACKS * NUM_STEPS)

#define NO_BUTTONS      0
#define BUTTON_1        1
#define BUTTON_2        2
#define BUTTON_1_2      3
#define BUTTON_3        4
#define BUTTON_1_3      5
#define BUTTON_2_3      6
#define BUTTON_1_2_3    7

typedef void (*Pot_cb)(int value); // type definition of callback functions for Pots


/////////////
// BUTTONS //
/////////////
class Buttons {
public:
    Buttons();
    ~Buttons(){}

    void read();

private:
    static int buttonPin[NUM_BUTTONS];
    int buttonIndex;
    int buttonRead;
    int buttonChange;
    int buttonState[NUM_BUTTONS];
    unsigned long buttonNow ;
    unsigned long buttonTime[NUM_BUTTONS];
    unsigned long *debounce_time;
    // unsigned long *debounce_time_default;
    // unsigned long *debounce_time_long;
    // const int debounceTime = 40;
    int *mode;
    int lastMode;
};


//////////
// KEYS //
//////////
class Keys {
public:
    Keys();
    ~Keys(){}

    void read();

private:
    static int keyPin[NUM_KEYS];
    int keyIndex;
    int keyRead;
    int keyValue;
    int keyState[NUM_KEYS];
    unsigned long keyNow;
    unsigned long keyTime[NUM_KEYS];
    unsigned long *debounce_time;
    int *keys;
    int *keyChange;
};


//////////
// POTS //
//////////
class Pots {
public:
    Pots();
    ~Pots(){}

    bool checkPot(int mode, int pot);
    void update();
    void attachCallback(int mode, int pot, Pot_cb callback);

private:
    Pot_cb pot_callback[NUM_MODES][NUM_POTS];
    static int potPin[NUM_POTS];
    int pot_hysteresis;
    int pot_values[NUM_MODES][NUM_POTS];
    int pot_hysteresis_center_value[NUM_POTS];
    int pot_beyond_hysteresis[NUM_POTS];
    int pot_lastMode[NUM_POTS];
};



//////////
// LEDS //
//////////
class Leds {
public:
    Leds();
    ~Leds(){}
    void update();
    void startupAnimation();

private:
    static int ledPin[NUM_LEDS];
    // int seqLed[NUM_LEDS];
    int statusLed = 13;
    int *indx;
    // int *last_indx;
    int leds;
    int chainedLedState;
    unsigned long ledNow;
    unsigned long ledTime;
    unsigned long ledPulse;
};



class Memory {
public:
    void wipe_eeprom();
private:
};


class Bodyseq {
public:
    Bodyseq();
    ~Bodyseq(){}

    void attachSequencer(Sequencer *s);
    void updatePosition();
    void update();
    void init();
    void playTrack();
    void selectNote();
    void selectStep();
    void selectOctave();
    void selectTrack();
    void chainTrack();
    void copyTrack();
    void clearTrack();
    void setupSequences();

    Buttons buttons;
    Keys keypads;
    Pots pots;
    Leds leds;
    Memory memory;
    Sequencer *sequencer;

    static int mode;
    static unsigned long debounce_time;
    static unsigned long debounce_time_long;
    static unsigned long debounce_time_default;
    static int keys;
    static int keychange;
    static int indx;
    static int last_indx;
    static int oct;
    static int slide;
    static int root_note;

    static int track_selected;
    static int step_selected;
    static int note_selected;
    static int octave_selected;
    static int track_playing;
    static int track_chained;

    static int octave[NUM_TRACKS * NUM_STEPS];
    static int slides[NUM_TRACKS * NUM_STEPS];
    int track[NUM_TRACKS];
    int notes[NUM_TRACKS * NUM_STEPS];
    int noteValues[8];
    int slideValues[8];
    int octaveValues[8];
    int note; // is this needed?

    const int scale[8] = {0, 2, 3, 5, 7, 8, 10, 12};

};


#endif // BODYSEQ_H
