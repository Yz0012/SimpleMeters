
#pragma once

#include <JuceHeader.h>

#include "../../CreateConfiguration/CreateColoursConfiguration.h"
#include "../../GUI/Configuration/ColourThemeEditor.h"

class ComponentThemeConfigurationButton : public juce::Component, public juce::SettableTooltipClient
{
public:
	ComponentThemeConfigurationButton(juce::String& categoryName);
	~ComponentThemeConfigurationButton();

	void paint(juce::Graphics&) override;
	void mouseDown(const juce::MouseEvent& event) override;
	void mouseEnter(const juce::MouseEvent& event) override;

	juce::String categoryName;
private:

};
