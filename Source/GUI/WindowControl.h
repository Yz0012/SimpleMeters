
#pragma once

#include <JuceHeader.h>

class WindowControl : public juce::Component , public juce::SettableTooltipClient
{
public:
	WindowControl();
	~WindowControl();

	void userTriedToCloseWindow() override;

	void paint(juce::Graphics&) override;

	void mouseDown(const juce::MouseEvent& event) override;
	void mouseEnter(const juce::MouseEvent& event) override;
private:
	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(WindowControl);
};