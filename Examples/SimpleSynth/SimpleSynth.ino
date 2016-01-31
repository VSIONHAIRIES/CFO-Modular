#include <SimpleSynth.h>

#define MIDI_CHANNEL 1

SimpleSynth synth;

int frequency = 220;
int cutoffModAmount = 0;
int cutoff = 0;
int wave = 0;

void setup() {

	usbMIDI.setHandleNoteOff(OnNoteOff);
	usbMIDI.setHandleNoteOn(OnNoteOn);
	usbMIDI.setHandleControlChange(OnControlChange);
	usbMIDI.setHandleRealTimeSystem(RealTimeSystem);

	synth.start();
	// saw.frequencyIn_ptr = &frequency;
	fltr.cutoffIn_ptr = &cutoff;
	fltr.cutoffModAmountIn_ptr = &cutoffModAmount;
	// put your setup code here, to run once:
	// env1.setVelSustain(0);
	env2.setVelSustain(0);
	wave1.setWaveform(DIGI1);
	wave2.setWaveform(FUZZ);
	wave2.setDetune(12);
	wave2.setSemitone(0);

}

void loop() {
	// midi.noteOut = 36 << 24;
	// midi.gateOut = SIGNED_BIT_32_HIGH;
	usbMIDI.read();

  // put your main code here, to run repeatedly:
	cutoff = analogRead(A0) << 21;
	cutoffModAmount = analogRead(A1) << 21;
	// wave = analogRead(A1) >> 6;
	// Serial.println(cutoff >> 24);

	// Serial.println(wave1.lowestIndx);
	// Serial.println(wave1.highestIndx);
	// static long cnt;
	// if(cnt > 48000) {
	// 	cnt =0;
	// 	Serial.print("midi.Noteout is ");
	// 	Serial.println(midi.noteOut >> 23);
	// 	Serial.print("*saw.frequencyIn_ptr is ");
	// 	Serial.println(*saw.frequencyIn_ptr >> 23);
	// } else {
	// 	cnt++;
	// }


}
