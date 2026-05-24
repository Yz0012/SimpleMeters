#pragma once

#include <JuceHeader.h>

#include "../../CreateConfiguration/CreateColoursConfiguration.h"

class VectorOscilloscopes : public juce::Component
{
public:
	VectorOscilloscopes();
	~VectorOscilloscopes();
	int bufferWritePosition = 0;
	void pushStereoBuffer(const juce::AudioBuffer<float>* localAudioBuffer);
	void paint (juce::Graphics&) override;

	const juce::AudioBuffer<float>* stereoBuffer;

	uint16_t callbackId = 0;
private:

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(VectorOscilloscopes);
};