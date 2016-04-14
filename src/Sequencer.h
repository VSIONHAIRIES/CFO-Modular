#ifndef SEQUENCER_H
#define SEQUENCER_H

#include "AudioNode.h"

#define MAX_SEQ 8
#define MAX_STEPS 8
#define INSTR_SEQ 4
#define ISEQ_NBR_STEPS 32

#define TICKS_PER_QUARTER_NOTE 24 //

enum SUBDIV {
    NOTE_0,
    NOTE_1 = (TICKS_PER_QUARTER_NOTE * 4),
    NOTE_1DOT = (TICKS_PER_QUARTER_NOTE * 3),
    NOTE_2 = (TICKS_PER_QUARTER_NOTE * 2),
    NOTE_3 = ((TICKS_PER_QUARTER_NOTE * 3) / 2),
    NOTE_4 = (TICKS_PER_QUARTER_NOTE * 1),
    NOTE_6 = ((TICKS_PER_QUARTER_NOTE * 2) / 3),
    NOTE_8 = (TICKS_PER_QUARTER_NOTE / 2),
    NOTE_12 = (TICKS_PER_QUARTER_NOTE / 3),
    NOTE_16 = (TICKS_PER_QUARTER_NOTE / 4),
    NOTE_24 = (TICKS_PER_QUARTER_NOTE / 6),
    NOTE_32 = (TICKS_PER_QUARTER_NOTE / 8),
    NOTE_48 = (TICKS_PER_QUARTER_NOTE / 12),
    NOTE_64 = (TICKS_PER_QUARTER_NOTE / 16),
    NOTE_96 = (TICKS_PER_QUARTER_NOTE / 24)
//    NOTE_128 = (TICKS_PER_QUARTER_NOTE / 32),
//    NOTE_192 = (TICKS_PER_QUARTER_NOTE / 48),
//    NOTE_256 = (TICKS_PER_QUARTER_NOTE / 64),
//    NOTE_384 = (TICKS_PER_QUARTER_NOTE / 96)
};

enum SEQ_LOOP_TYPE {
    ONCE = 0,
    LOOP = 1,
    PINGPONG = 2    // not implemented yet
};

#define REVERSE true

typedef void (*sequencerClock_cb)(void);

typedef void (*func_cb)(void);

class seq;

class Sequencer : public AudioNode {
  friend class seq;
  public:
    Sequencer();
    Sequencer(int bpm);
    ~Sequencer(){}

    void init(int bpm);
    void update();
    void internalClock();

    void attachSequencerClockCallbackClock(sequencerClock_cb cbClock);
  	void attachSequencerClockCallbackStart(sequencerClock_cb cbStart);
  	void attachSequencerClockCallbackStop(sequencerClock_cb cbStop);
  	void attachSequencerClockCallbackContinue(sequencerClock_cb cbContinue);



    void sendClock();
    void sendStart();
    void sendContinue();
    void sendStop();

    void clock();
    void stop();
    void start();
    void continues();

    int newSequence(SUBDIV subdiv, func_cb cb);
    int newSequence(SUBDIV subdiv, int steps, int channel);
    int newSequence(SUBDIV subdiv, int steps, SEQ_LOOP_TYPE loop);
    int newSequence(SUBDIV subdiv, int steps, SEQ_LOOP_TYPE loop, bool reverse);

    bool stopSequence(int index);
    bool startSequence(int index);
    bool continueSequence(int index);

    void setInternalClock(bool i);
    bool getInternalClock();

    void setbpm(int bpm);
    int getbpm();

    bool setChannel(int index, int c);
    int getChannel(int index);

    bool setSteps(int index, int s);
    int getSteps(int index);

    bool setPosition(int index, int p);
    int getPosition(int index);

    bool setBegin(int index, int b);
    int getBegin(int index);

    bool setEnd(int index, int e);
    int getEnd(int index);

    bool setReverse(int index, bool r);
    bool getReverse(int index);

    bool setInternal(int index, bool i);
    bool getInternal(int index);

    bool setExternal(int index, bool e);
    bool getExternal(int index);

    bool setSubdiv(int index, SUBDIV subdiv);
    void setSubdivAllSeqs(SUBDIV subdiv);
    int getSubdiv(int index);
    void setSubdivByIndex(int index, int subdiv_i);
    void setSubdivAllSeqsByIndex(int subdiv_i);

    bool setLoopType(int index, SEQ_LOOP_TYPE loop);
    int getLoopType(int index);

    bool setCallback(int index, func_cb cb);
    func_cb getCallback(int index);

    bool insertNotes(int index, int notes[], int numNotes, int newPosition);

    int setSelectedSequence(int s);
    // int getSelectedSequence();

    void setGatewidth(int index, int gw);
    void setGatewidthAllSeqs(int gw);

    // void enable_restart(bool restart) { restart_enabled = restart; }
    // bool get_restart() { return restart_enabled; }

    void set_restart_countdown(int number);
    void reset_restart_countdown();

    int noteOut;
    int gateOut;

    int noteOffOut[4];
    int noteOnOut[4];

    bool* clockIn_ptr;
    bool* startIn_ptr;
    bool* continueIn_ptr;
    bool* stopIn_ptr;
    bool clockOut;
    bool startOut;
    bool continueOut;
    bool stopOut;

protected:

    seq* _sequences[MAX_SEQ];
    int _bpm;
    int _bpmInClockTicks;
    bool _internalClock;
    unsigned long clockTick;
    unsigned long timeNow;
    unsigned long lastTime;
    unsigned long tickTime;

    int selectedSequence;
    int selectedPosition;

    // function pointers for passing MIDI Clock on to MIDI class
    sequencerClock_cb sequencerClockCB_clock;
    sequencerClock_cb sequencerClockCB_start;
    sequencerClock_cb sequencerClockCB_stop;
    sequencerClock_cb sequencerClockCB_continue;

    bool restart_enabled;
    int restart_countdown;
    int restart_counter;
    bool _clockIn;
    bool _startIn;
    bool _continueIn;
    bool _stopIn;

};

class seq {
  friend class Sequencer;

  private:

    seq(int id, SUBDIV subdiv, func_cb cb);
    seq(int id, SUBDIV subdiv, int steps, int channel);
    seq(int id, SUBDIV subdiv, int steps, SEQ_LOOP_TYPE loop);
    seq(int id, SUBDIV subdiv, int steps, SEQ_LOOP_TYPE loop, bool reverse);

    int _id;
    int _channel;
    int _steps;
    int _begin;
    int _end;
    int _position;
    int _lastposition;
    bool _reverse;
    bool _stopped;
    bool _internal;
    bool _external;
    SUBDIV _subdiv;
    int _gatewidth;
    SEQ_LOOP_TYPE _loop;


    int _notes[MAX_STEPS];
    int _velocity[MAX_STEPS];
    int _ccNumbers[MAX_STEPS];
    int _ccValues[MAX_STEPS];

    unsigned long step;

    void trigger(int *noteout_ptr, int *gateout_ptr); //, int *noteoffout_ptr, int *noteonout_ptr);
    void triggerNoteOff(int *noteout_ptr, int *gateout_ptr); //, int *noteoffout_ptr, int *noteonout_ptr);
    void triggerNoteOn(int *noteout_ptr, int *gateout_ptr); //, int *noteoffout_ptr, int *noteonout_ptr);

    void setchannel(int c);
    int getchannel();

    void setsteps(int s);
    int getsteps();

    void setcurrentstep(int s);
    int getcurrentstep();

    void setposition(int p);
    int getposition();

    void setbegin(int b);
    int getbegin();

    void setend(int e);
    int getend();

    void setreverse(bool r);
    bool getreverse();

    void setinternal(bool i);
    bool getinternal();

    void setexternal(bool e);
    bool getexternal();

    void setsubdiv(SUBDIV sd);
    SUBDIV getsubdiv();

    void setgatewidth(int gw);
    int getgatewidth();

    void setlooptype(SEQ_LOOP_TYPE loop);
    SEQ_LOOP_TYPE getlooptype();

    void callback(func_cb cb);

    func_cb _callback;

    void insertnotes(int notes[], int numNotes, int newPosition);
};

#endif // SEQUENCER_H
