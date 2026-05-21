
#pragma once

#include <JuceHeader.h>

class WindowControl : public juce::Component
{
public:
	WindowControl();
	~WindowControl();

	void userTriedToCloseWindow() override;

	void paint(juce::Graphics&) override;

	void mouseDown(const juce::MouseEvent& event) override;
	void mouseEnter(const juce::MouseEvent& event) override;
private:
};