
#pragma once

#include <JuceHeader.h>

class PinOnTopButton : public juce::Component, public juce::SettableTooltipClient
{
public:
	PinOnTopButton();
	~PinOnTopButton();

	using Callback = std::function<void()>;

	void paint(juce::Graphics&) override;

	void mouseDown(const juce::MouseEvent& event) override;
	void mouseEnter(const juce::MouseEvent& event) override;

	void callBackFunction();
	Callback onClick = nullptr;
	bool pinFixed = false;
private:
	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PinOnTopButton);
};