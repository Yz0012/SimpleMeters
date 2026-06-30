
#pragma once

#include <JuceHeader.h>

class BufferSizeButton : public juce::Component, public juce::SettableTooltipClient
{
public:
	BufferSizeButton();
	~BufferSizeButton();

	void paint(juce::Graphics&) override;

	void mouseEnter(const juce::MouseEvent& event) override;
private:
	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(BufferSizeButton);
};

