
#pragma once

#include <JuceHeader.h>

class WindowsSizeButton : public juce::Component ,public juce::SettableTooltipClient
{
	using Callback = std::function<void()>;

public:
	WindowsSizeButton();
	~WindowsSizeButton();

	void paint(juce::Graphics&) override;

	void mouseDown(const juce::MouseEvent& event) override;
	void mouseEnter(const juce::MouseEvent& event) override;

	void callbackFunction() const;

	Callback onClick;

	bool isOpen = false;
private:
	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(WindowsSizeButton);
};