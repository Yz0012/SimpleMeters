
#pragma once

#include <JuceHeader.h>

class HeaderFixedButton : public juce::Component
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
};