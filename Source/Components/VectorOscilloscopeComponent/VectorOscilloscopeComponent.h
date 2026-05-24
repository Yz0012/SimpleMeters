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

	uint16_t callbackId = 0;
private:
	const juce::AudioBuffer<float>* stereoBuffer = nullptr;

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(VectorOscilloscopes);
};