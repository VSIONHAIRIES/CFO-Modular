#include <AudioNode.h>
#include <Arduino.h>

AudioNode::AudioNode() {
	_audioIn = 0;
	_audioOut = 0;
	audioOut = 0;
	
	_noSignal = 0; 
}

int AudioNode::convertFloatInput(float value) {
	int64_t v = int64_t(value * float(SIGNED_BIT_32_HIGH));
	if(v > SIGNED_BIT_32_HIGH) v = SIGNED_BIT_32_HIGH;
	if(v < SIGNED_BIT_32_LOW) v = SIGNED_BIT_32_LOW;
	return int(v);
}
