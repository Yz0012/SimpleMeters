
#pragma once

#include <JuceHeader.h>

class WASAPIButton : public juce::Component, public juce::SettableTooltipClient
{

	using Callback = std::function<void()>;

public:
	WASAPIButton();
	~WASAPIButton();

	void paint(juce::Graphics&) override;

	void mouseDown(const juce::MouseEvent& event) override;
	void mouseEnter(const juce::MouseEvent& event) override;

	void callbackFunction() const;

	Callback onClick;

	bool isOpen = false;
private:
};
