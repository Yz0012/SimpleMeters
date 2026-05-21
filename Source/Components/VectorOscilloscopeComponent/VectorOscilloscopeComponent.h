#pragma once

#include <JuceHeader.h>

#include "../../CreateConfiguration/CreateColoursConfiguration.h"

class VectorOscilloscopes : public juce::Component
{
public:
	VectorOscilloscopes();
	~VectorOscilloscopes();
	juce::AudioBuffer<float> stereoBuffer{ 2, 1024 };
	int bufferWritePosition = 0;
	void createStereoBufferAndPaint(float L, float R);
	void paint (juce::Graphics&);

	float x;
	float y;

	uint16_t callbackId;
private:

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(VectorOscilloscopes);
};