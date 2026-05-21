
#pragma once

#include <JuceHeader.h>

class BufferSizeButton : public juce::Component, public juce::SettableTooltipClient
{
public:
	BufferSizeButton();
	~BufferSizeButton();

	void paint(juce::Graphics&) override;

	//void mouseDown(const juce::MouseEvent& event) override;
	void mouseEnter(const juce::MouseEvent& event) override;
private:

};

