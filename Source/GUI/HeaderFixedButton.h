
#pragma once

#include <JuceHeader.h>

class HeaderFixedButton : public juce::Component, public juce::SettableTooltipClient
{
public:
	HeaderFixedButton();
	~HeaderFixedButton();

	void paint(juce::Graphics&) override;
	void mouseDown(const juce::MouseEvent& event) override;
	void mouseEnter(const juce::MouseEvent& event) override;

	bool getHeaderFixed() const;
private:
	bool headerFixed = false;

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(HeaderFixedButton);
};