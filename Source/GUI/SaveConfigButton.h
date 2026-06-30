
#pragma once

#include <JuceHeader.h>

#include "../CreateConfiguration/StartUpConfiguration.h"

class SaveConfigButton : public juce::Component, public juce::SettableTooltipClient
{
public:
	SaveConfigButton();
	~SaveConfigButton();

	void paint(juce::Graphics&) override;
	void mouseDown(const juce::MouseEvent& event) override;
	void mouseEnter(const juce::MouseEvent& event) override;
	void valueTreePropertyChanged();
private:
	bool configSaved = true;

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SaveConfigButton);
};