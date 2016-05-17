#include <stdlib.h>
#include <Arduino.h>
#include "Sequencer.h"
// #include "BodyseqSynth.h"
// #include <spi4teensy3.h>

// Sequencer Sequencer;

Sequencer::Sequencer() {
    setbpm(120);
    setPortamento(0);
    clockTick = 0;
    for(int i = 0; i < MAX_SEQ; i++) {
        _sequences[i] = NULL;
    }
    set_restart_countdown(0);
    reset_restart_countdown();
}

Sequencer::Sequencer(int bpm) {
    setbpm(bpm);
    setPortamento(0);
    clockTick = 0;
    for(int i = 0; i < MAX_SEQ; i++) {
        _sequences[i] = NULL;
    }
    set_restart_countdown(0);
    reset_restart_countdown();
}


void Sequencer::init(int bpm) {
    setbpm(bpm);
    setPortamento(0);
    clockTick = 0;
    for(int i = 0; i < MAX_SEQ; i++) {
        _sequences[i] = NULL;
    }
    set_restart_countdown(0);
    reset_restart_countdown();
}


void Sequencer::update()
{
    if(getInternalClock()) {
        internalClock();
    }

    for(int i = 0; i < MAX_SEQ; i++) {
        seq* s = _sequences[i];
        if(s == NULL || s->_stopped) continue;
        if(clockTick >= s -> step) {
            if(s -> _steps) {
                //  Serial.println("_steps");
                s -> triggerNoteOn(&noteOut, &gateOut); // NOTEON
                //  Serial.println("triggerNoteOn");
            } else {
                s->_callback();
                //  Serial.println("callback");
            }
            s->step += s -> _subdiv;
            // Serial.println("s->step += s -> _subdiv");
            // Serial.print("s->step is now ");
            // Serial.println(s -> getcurrentstep());
        }
        if(clockTick == ((s -> step) - (s -> _subdiv) + (s -> _gatewidth))) {
            if(s -> _steps) {
                //  Serial.println("_steps");
                s -> triggerNoteOff(&noteOut, &gateOut);  // NOTEOFF
                //  Serial.println("triggerNoteOff");
            } else {
                // s->_callback(); // NEED A WAY TO SEND GATE_OFF CALLBACKS
                //  Serial.println("callback");
            }
        }
    }
}


void Sequencer::internalClock()
{
    timeNow = micros();
    if(timeNow - lastTime >= tickTime) {
        if(clockTick >= NOTE_1) {
            start();
            sendStart();
        }
        clock();
        sendClock();
        lastTime = timeNow;
    }
}


void Sequencer::setInternalClock(bool i)
{
    _internalClock = i;
}


bool Sequencer::getInternalClock()
{
    return _internalClock;
}


void Sequencer::clock()
{
    clockTick++;
}


void Sequencer::start()
{
    if(restart_counter < 1) {
        // if(clockTick && !restart_enabled) return;
        clockTick = 0;
        for(int i = 0; i < MAX_SEQ; i++) {
            startSequence(i);
        }
        reset_restart_countdown();
    } else {
        restart_counter -= 1;
    }
}


void Sequencer::continues()
{
    clockTick = 0;  // is this right????
    for(int i = 0; i < MAX_SEQ; i++) {
        continueSequence(i);
    }
}


void Sequencer::stop()
{
    for(int i = 0; i < MAX_SEQ; i++) {
        stopSequence(i);
        seq* s = _sequences[i];
        if(s == NULL) continue;
        s -> triggerNoteOff(&noteOut, &gateOut);  // NOTEOFF
    }
    // sendStop();
}


void Sequencer::attachSequencerClockCallbackClock(sequencerClock_cb cbClock)
{
    sequencerClockCB_clock = cbClock;
}


void Sequencer::attachSequencerClockCallbackStart(sequencerClock_cb cbStart)
{
    sequencerClockCB_start = cbStart;
}


void Sequencer::attachSequencerClockCallbackStop(sequencerClock_cb cbStop)
{
    sequencerClockCB_stop = cbStop;
}


void Sequencer::attachSequencerClockCallbackContinue(sequencerClock_cb cbContinue)
{
    sequencerClockCB_continue = cbContinue;
}


void Sequencer::sendClock()
{
    sequencerClockCB_clock();
}


void Sequencer::sendStart()
{
    sequencerClockCB_start();
}


void Sequencer::sendContinue()
{
    sequencerClockCB_stop();
}


void Sequencer::sendStop()
{
    sequencerClockCB_continue();
}


int Sequencer::newSequence(SUBDIV subdiv, func_cb cb)
{
    int j = -1;
    for(int i = 0; i < MAX_SEQ; i++) {
        if(_sequences[i] == NULL) {
            j = i;
            break;
        }
    }

    if(j >= 0) {
        seq* s = new seq(j, subdiv, cb);
        _sequences[j] = s;
        Serial.print("Created sequence ");
        Serial.println(j);
    }

    return j;
}


int Sequencer::newSequence(SUBDIV subdiv, int steps, SEQ_LOOP_TYPE loop)
{
    int j = -1;
    for(int i = 0; i < MAX_SEQ; i++) {
        if(_sequences[i] == NULL) {
            j = i;
            break;
        }
    }

    if(j >= 0) {
        seq* s = new seq(j, subdiv, steps, loop);
        _sequences[j] = s;
        Serial.print("Created sequence ");
        Serial.println(j);
    }

    return j;
}


int Sequencer::newSequence(SUBDIV subdiv, int steps, int channel)
{
    int j = -1;
    for(int i = 0; i < MAX_SEQ; i++) {
        if(_sequences[i] == NULL) {
            j = i;
            break;
        }
    }

    if(j >= 0) {
        seq* s = new seq(j, subdiv, steps, channel);
        _sequences[j] = s;
        Serial.print("Created sequence ");
        Serial.println(j);
    }

    return j;
}


int Sequencer::newSequence(SUBDIV subdiv, int steps, SEQ_LOOP_TYPE loop, bool reverse)
{
    int j = -1;
    for(int i = 0; i < MAX_SEQ; i++) {
        if(_sequences[i] == NULL) {
            j = i;
            break;
        }
    }

    if(j >= 0) {
        seq* s = new seq(j, subdiv, steps, loop, reverse);
        _sequences[j] = s;
        Serial.print("Created sequence ");
        Serial.println(j);
    }

    return j;
}


bool Sequencer::stopSequence(int index)
{
    if(index >= 0 && index < MAX_SEQ && _sequences[index] != NULL) {
        _sequences[index]->_stopped = true;
        return true;
    }
    return false;
}


bool Sequencer::startSequence(int index)
{
    if(index >= 0 && index < MAX_SEQ && _sequences[index] != NULL) {

        bool reverse = _sequences[index] ->  _reverse;
        if(reverse) {
            _sequences[index] -> _position = _sequences[index] -> _begin;
        } else {
            _sequences[index] -> _position = _sequences[index] -> _end;
        }
        _sequences[index] -> step = 0;
        _sequences[index] -> _stopped = false;

        return true;
    }
    return false;
}


bool Sequencer::continueSequence(int index)
{
    if(index >= 0 && index < MAX_SEQ && _sequences[index] != NULL) {
        _sequences[index]->_stopped = false;
        return true;
    }
    return false;
}


void Sequencer::setbpm(int bpm)
{
    if(bpm > 0){
        _bpm = bpm;
        _bpmInClockTicks = _bpm * TICKS_PER_QUARTER_NOTE;
        tickTime = 60 * 1000000 / _bpmInClockTicks;
    }
}


void Sequencer::setPortamento(int portamento)
{
    _portamento =  portamento;
}


int Sequencer::getPortamento()
{
    return _portamento;
}


int Sequencer::getbpm()
{
    return _bpm;
}


bool Sequencer::setChannel(int index, int channel)
{
    if(index >= 0 && index < MAX_SEQ && _sequences[index] != NULL) {
        //        _sequences[index]->_channel;
        _sequences[index]->setchannel(channel);
        return true;
    }
    return false;
}


int Sequencer::getChannel(int index)
{
    if(index >= 0 && index < MAX_SEQ && _sequences[index] != NULL) {
        return _sequences[index]->_channel;
        //        return _sequences[index]->getchannel();
    }
    return -1;
}


bool Sequencer::setSteps(int index, int steps)
{
    if(index >= 0 && index < MAX_SEQ && _sequences[index] != NULL) {
        _sequences[index]->setsteps(steps);
        return true;
    }
    return false;
}


int Sequencer::getSteps(int index)
{
    if(index >= 0 && index < MAX_SEQ && _sequences[index] != NULL) {
        return _sequences[index]->getsteps();
    }
    return -1;
}


bool Sequencer::setPosition(int index, int position)
{
    if(index >= 0 && index < MAX_SEQ && _sequences[index] != NULL) {
        _sequences[index]->setposition(position);
        return true;
    }
    return false;
}


int Sequencer::getPosition(int index)
{
    if(index >= 0 && index < MAX_SEQ && _sequences[index] != NULL) {
        return _sequences[index]->getposition();
    }
    return -1;
}


bool Sequencer::setBegin(int index, int begin)
{
    if(index >= 0 && index < MAX_SEQ && _sequences[index] != NULL) {
        _sequences[index]->setbegin(begin);
        return true;
    }
    return false;
}


int Sequencer::getBegin(int index)
{
    if(index >= 0 && index < MAX_SEQ && _sequences[index] != NULL) {
        return _sequences[index]->getbegin();
    }
    return -1;
}


bool Sequencer::setEnd(int index, int end)
{
    if(index >= 0 && index < MAX_SEQ && _sequences[index] != NULL) {
        _sequences[index]->setend(end);
        return true;
    }
    return false;
}


int Sequencer::getEnd(int index)
{
    if(index >= 0 && index < MAX_SEQ && _sequences[index] != NULL) {
        return _sequences[index]->getend();
    }
    return -1;
}


bool Sequencer::setReverse(int index, bool reverse)
{
    if(index >= 0 && index < MAX_SEQ && _sequences[index] != NULL) {
        _sequences[index]->setreverse(reverse);
        return true;
    }
    return false;
}


bool Sequencer::getReverse(int index)
{
    if(index >= 0 && index < MAX_SEQ && _sequences[index] != NULL) {
        return _sequences[index]->getreverse();
    }
    return -1;
}


bool Sequencer::setInternal(int index, bool internal)
{
    if(index >= 0 && index < MAX_SEQ && _sequences[index] != NULL) {
        _sequences[index]->setinternal(internal);
        return true;
    }
    return false;
}


bool Sequencer::getInternal(int index)
{
    if(index >= 0 && index < MAX_SEQ && _sequences[index] != NULL) {
        return _sequences[index]->getinternal();
    }
    return -1;
}


bool Sequencer::setExternal(int index, bool external)
{
    if(index >= 0 && index < MAX_SEQ && _sequences[index] != NULL) {
        _sequences[index]->setexternal(external);
        return true;
    }
    return false;
}


bool Sequencer::getExternal(int index)
{
    if(index >= 0 && index < MAX_SEQ && _sequences[index] != NULL) {
        return _sequences[index]->getexternal();
    }
    return -1;
}


bool Sequencer::setSubdiv(int index, SUBDIV subdiv)
{
    if(index >= 0 && index < MAX_SEQ && _sequences[index] != NULL) {
        _sequences[index]->setsubdiv(subdiv);
        return true;
    }
    return false;
}


void Sequencer::setSubdivAllSeqs(SUBDIV subdiv) {
    int index = 0;
    while(index < MAX_SEQ && _sequences[index] != NULL) {
        _sequences[index]->setsubdiv(subdiv);
        index++;
    }
}


void Sequencer::setSubdivAllSeqsByIndex(int subdiv_i) {
    int index = 0;
    while(index < MAX_SEQ && _sequences[index] != NULL) {
        setSubdivByIndex(index, subdiv_i);
        index++;
    }
}


void Sequencer::setSubdivByIndex(int index, int subdiv_i) {
    if(subdiv_i > 0 && subdiv_i < 15) {
        SUBDIV subdivIndex[] = {NOTE_0, NOTE_1, NOTE_1DOT, NOTE_2, NOTE_3, NOTE_4, NOTE_6, NOTE_8, NOTE_12, NOTE_16, NOTE_24, NOTE_32, NOTE_48, NOTE_64, NOTE_96};
        if(index < MAX_SEQ && _sequences[index] != NULL) {
            _sequences[index]->setsubdiv(subdivIndex[subdiv_i]);
            index++;
        }
    }
}


int Sequencer::getSubdiv(int index)
{
    if(index >= 0 && index < MAX_SEQ && _sequences[index] != NULL) {
        return _sequences[index]->getsubdiv();
    }
    return -1;
}


bool Sequencer::setLoopType(int index, SEQ_LOOP_TYPE loop)
{
    if(index >= 0 && index < MAX_SEQ && _sequences[index] != NULL) {
        _sequences[index]->setlooptype(loop);
        return true;
    }
    return false;
}


int Sequencer::getLoopType(int index)
{
    if(index >= 0 && index < MAX_SEQ && _sequences[index] != NULL) {
        return _sequences[index]->getlooptype();
    }
    return -1;
}


bool Sequencer::setCallback(int index, func_cb cb)
{
    if(index >= 0 && index < MAX_SEQ && _sequences[index] != NULL) {
        _sequences[index]->callback(cb);
        return true;
    }
    return false;
}


func_cb Sequencer::getCallback(int index)
{
    if(index >= 0 && index < MAX_SEQ && _sequences[index] != NULL) {
        return _sequences[index]->_callback;
    }
    return NULL;
}



bool Sequencer::insertNotes(int index, int notes[], int numNotes, int newPosition)
{
    if(index >= 0 && index < MAX_SEQ && _sequences[index] != NULL) {
        _sequences[index]->insertnotes(notes, numNotes, newPosition);
        return true;
    }
    return false;
}


bool Sequencer::insertSlides(int index, int slides[], int numSlides, int newPosition)
{
    if(index >= 0 && index < MAX_SEQ && _sequences[index] != NULL) {
        _sequences[index]->insertslides(slides, numSlides, newPosition);
        return true;
    }
    return false;
}


int Sequencer::setSelectedSequence(int s)
{
    if(s < 0) s = 0;
    if(s > MAX_SEQ) s = MAX_SEQ - 1;
    selectedSequence = s;
    return selectedSequence;
}


void Sequencer::setGatewidth(int index, int gw) {
    _sequences[index]->setgatewidth(gw);
}


void Sequencer::setGatewidthAllSeqs(int gw) {
    int index = 0;
    while(index < MAX_SEQ && _sequences[index] != NULL) {
        _sequences[index]->setgatewidth(gw);
        index++;
    }
}


void Sequencer::set_restart_countdown(int number) {
    restart_countdown = number;
    // reset_restart_countdown();
}


void Sequencer::reset_restart_countdown() {
    restart_counter = restart_countdown;
}










// seq

seq::seq(int id, SUBDIV subdiv, func_cb cb) : _id(id), _stopped(true)
{
    setsubdiv(subdiv);
    callback(cb);
    setcurrentstep(0);
    setsteps(0);
}


seq::seq(int id, SUBDIV subdiv,  int steps, int channel) : _id(id), _stopped(true)
{
    setchannel(channel);
    setsubdiv(subdiv);
    setgatewidth(64);
    setsteps(steps);
    setlooptype(LOOP);
    setreverse(false);
    setposition(0);
    setbegin(0);
    if(steps <= MAX_STEPS) setend(steps-1);
    else setend(MAX_STEPS);
    for(int i = 0; i < MAX_STEPS; i++) {
        _notes[i] = 36 + 3 * i;
        _velocity[i] = 127;
        _slides[i] = 0;
    }
    setinternal(true);
    setexternal(true);
    _lastposition = 0;

}


seq::seq(int id, SUBDIV subdiv,  int steps, SEQ_LOOP_TYPE loop) : _id(id), _stopped(true)
{
    // setchannel(Midi.midiChannel);
    setsubdiv(subdiv);
    setgatewidth(64);
    setsteps(steps);
    setlooptype(loop);
    setreverse(false);
    setposition(0);
    setbegin(0);
    if(steps <= MAX_STEPS) setend(steps-1);
    else setend(MAX_STEPS);
    for(int i = 0; i < MAX_STEPS; i++) {
        _notes[i] = 36 + 3 * i;
        _velocity[i] = 127;
        _slides[i] = 0;
    }
    setinternal(true);
    setexternal(true);
    _lastposition = 0;
}


seq::seq(int id, SUBDIV subdiv,  int steps, SEQ_LOOP_TYPE loop, bool reverse) : _id(id), _stopped(true)
{
    // setchannel(Midi.midiChannel);
    setsubdiv(subdiv);
    setgatewidth(64);
    setsteps(steps);
    setlooptype(loop);
    if(reverse) {
        setreverse(true);
        setposition(steps-1);
    } else {
        setreverse(false);
        setposition(0);
    }
    setbegin(0);
    if(steps <= MAX_STEPS) setend(steps-1);
    else setend(MAX_STEPS);
    for(int i = 0; i < MAX_STEPS; i++) {
        _notes[i] = 36 + 3 * i;
        _velocity[i] = 127;
        _slides[i] = 0;
    }
    setinternal(true);
    setexternal(true);
    _lastposition = 0;
}


void seq::trigger(int *noteout_ptr, int *gateout_ptr) //, int *noteoffout_ptr, int *noteonout_ptr)
{
    if(_begin < 0 ) {
        // Serial.print("_begin is ");
        // Serial.println(_begin);
        _begin = 0;
    }
    //    Serial.print("_end is ");
    //    Serial.println(_end);
    //    Serial.print("_position is ");
    //    Serial.println(_position);
    //    Serial.println("enter trigger");

    if(_internal) {
        // Serial.println("enter conditional");
        // Midi.noteOff(_channel, _notes[_lastposition], _velocity[_lastposition]);
        // Serial.println("internal noteOff"); // NOT TRIGGERED IN THIS IMPLEMENTATION?

        // Midi.noteOn(_channel, _notes[_position], _velocity[_position]);
        // Serial.println("internal noteOff");
        *noteout_ptr = _notes[_position] << 24;
        *gateout_ptr = SIGNED_BIT_32_HIGH;
    }
    if(_external) {
        // Midi.sendNoteOff(_channel, _notes[_lastposition], _velocity[_lastposition]);
        // Serial.println("external noteOff");
        // *noteoffout_ptr = 1;
        // *(noteoffout_ptr + 1) = _channel;
        // *(noteoffout_ptr + 2) = _notes[_lastposition];
        // *(noteoffout_ptr + 3) = _velocity[_lastposition];

        // Midi.sendNoteOn(_channel, _notes[_position], _velocity[_position]);
        // Serial.println("external noteOff");
        // *noteonout_ptr = 1;
        // *(noteonout_ptr + 1) = _channel;
        // *(noteonout_ptr + 2) = _notes[_position];
        // *(noteonout_ptr + 3) = _velocity[_position];
    }

    _lastposition = _position;

    if(_reverse) {
        if(_position <= _begin) {
            _position = _end + 1;
            if(!_loop) _stopped = true;
        }
        _position--;
    } else {
        if(_position >= _end) {
            _position = _begin - 1;
            if(!_loop) _stopped = true;
        }
        _lastposition = _position;
        //        Serial.print("_last position is ");
        //        Serial.println(_lastposition);
        //        Serial.println();
        _position++;
        //        Serial.print("_position is ");
        //        Serial.println(_position);
        //        Serial.println();

    }
    //    Serial.println("triggered");
}


void seq::triggerNoteOn(int *noteout_ptr, int *gateout_ptr) //, int *noteoffout_ptr, int *noteonout_ptr)
{

    if(_begin < 0 ) _begin = 0;

    if(_reverse) {
        if(_position <= _begin) {
            _position = _end + 1;
            if(!_loop) _stopped = true;
        }
        _position--;
    } else {
        if(_position >= _end) {
            _position = _begin - 1;
            if(!_loop) _stopped = true;
        }
        // _lastposition = _position;
        _position++;
    }

    if(_internal) {
        int note = _notes[_position];
        int slide = _slides[_position];
        if(note) {
            *noteout_ptr = note << 24;
            if(!slide) *gateout_ptr = SIGNED_BIT_32_HIGH;
        } else {
            *gateout_ptr = SIGNED_BIT_32_LOW;
        }
        // *noteout_ptr = _notes[_position] << 24;
    }
    if(_external) {
        // see seq::trigger(int*, int*) function for comments on what to put here
    }

    _lastposition = _position;

}


void seq::triggerNoteOff(int *noteout_ptr, int *gateout_ptr) //, int *noteoffout_ptr, int *noteonout_ptr)
{
    if(_begin < 0 ) _begin = 0;

    if(_internal) {
        // Serial.print("_position is: ");
        // Serial.println(_notes[_lastposition]);
        *noteout_ptr = _notes[_lastposition] << 24;
        *gateout_ptr = SIGNED_BIT_32_LOW;
    }
    if(_external) {
        // see seq::trigger(int*, int*) function for comments on what to put here
    }

    // _lastposition = _position;
    //
    // if(_reverse) {
    //   if(_position <= _begin) {
    //     _position = _end + 1;
    //     if(!_loop) _stopped = true;
    //   }
    //   _position--;
    // } else {
    //   if(_position >= _end) {
    //     _position = _begin - 1;
    //     if(!_loop) _stopped = true;
    //   }
    //   _lastposition = _position;
    //   _position++;
    // }
}


void seq::insertnotes(int notes[], int numNotes, int newPosition)
{
    for(int i = 0; i < numNotes; i++) {
        int pos = newPosition + i;
        int note = notes[i];
        if(note > 127) note = 127;
        else if(note < 0 ) note = 0;
        if((pos >= 0) && (pos < MAX_STEPS)) {
            _notes[pos] = note;
        }
    }
}


void seq::insertslides(int slides[], int numSlides, int newPosition)
{
    for(int i = 0; i < numSlides; i++) {
        int pos = newPosition + i;
        int slide = slides[i];
        if(slide > 0) slide = 1;
        else if(slide < 0 ) slide = 0;
        if((pos >= 0) && (pos < MAX_STEPS)) {
            _slides[pos] = slide;
        }
    }
}


void seq::setchannel(int c)
{
    _channel = c;
}


int seq::getchannel()
{
    return _channel;
}


void seq::setsteps(int s)
{
    _steps = s;
}


int seq::getsteps()
{
    return _steps;
}


void seq::setcurrentstep(int s)
{
    step = s;
}


int seq::getcurrentstep()
{
    return step;
}


void seq::setposition(int p)
{
    _position = p;
}


int seq::getposition()
{
    return _position;
}


void seq::setbegin(int b)
{
    if(b < 0) b = 0;
    if(b > _end) b = _end;
    if(b >= _steps) b = _steps - 1;
    _begin = b;
}


int seq::getbegin()
{
    return _begin;
}


void seq::setend(int e)
{
    if(e < 0) e = 0;
    if(e >= _steps) e = _steps - 1;
    if(e < _begin) e = _begin;
    _end = e;
}


int seq::getend()
{
    return _end;
}


void seq::setreverse(bool r)
{
    _reverse = r;
}


bool seq::getreverse()
{
    return _reverse;
}


void seq::setinternal(bool i)
{
    _internal = i;
}


bool seq::getinternal()
{
    return _internal;
}


void seq::setexternal(bool e)
{
    _external = e;
}


bool seq::getexternal()
{
    return _external;
}


void seq::setsubdiv(SUBDIV v)
{
    _subdiv = v;
}


SUBDIV seq::getsubdiv()
{
    return _subdiv;
}


void seq::setgatewidth(int gw)
{
    if(gw < 0) gw = 0;
    if(gw > 127) gw = 127;
    gw = (_subdiv * gw) >> 6;
    gw = (gw >> 1) + (gw & 1);
    _gatewidth = gw;
}


int seq::getgatewidth()
{
    return _gatewidth;
}


void seq::setlooptype(SEQ_LOOP_TYPE loop)
{
    _loop = loop;
}


SEQ_LOOP_TYPE seq::getlooptype()
{
    return _loop;
}


void seq::callback(func_cb cb)
{
    _callback = cb;
}
