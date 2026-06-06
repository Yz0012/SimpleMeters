#pragma once

#include <JuceHeader.h>

#include "../../CreateConfiguration/CreateColoursConfiguration.h"
#include "../../GUI/Components/DrawBounds.h"
#include "../../GUI/Components/ComponentHeader.h"
#include "OscilloscopeReferenceLines.h"

class VectorOscilloscopes : public juce::Component, private juce::ValueTree::Listener
{
public:
	VectorOscilloscopes();
	~VectorOscilloscopes();
	int bufferWritePosition = 0;
	void pushStereoBuffer(const juce::AudioBuffer<float>* localAudioBuffer);
	void paint (juce::Graphics&) override;
	void mouseDown(const juce::MouseEvent& event) override;
	void mouseEnter(const juce::MouseEvent& event) override;
	void valueTreePropertyChanged(juce::ValueTree& tree, const juce::Identifier& property) override;

	void mouseExit(const juce::MouseEvent&) override;

	uint16_t callbackId = 0;

	ComponentHeader componentHeader{ juce::String("VectorOscilloscopes") };
	DrawBounds drawBounds;
	OscilloscopeReferenceLines oscilloscopeReferenceLines;

	using Callback = std::function<void()>;
	Callback cb = nullptr;
private:
	const juce::AudioBuffer<float>* stereoBuffer = nullptr;

	juce::ValueTree vectorCat;

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(VectorOscilloscopes);
};