
#pragma once

#include <JuceHeader.h>

#include "../CreateConfiguration/CreateColoursConfiguration.h"
#include "../GUI/Configuration/ColourThemeEditor.h"

class ThemeConfigurationButton : public juce::Component
{
public:
	ThemeConfigurationButton();
	~ThemeConfigurationButton();

	void paint(juce::Graphics&) override;
	void mouseDown(const juce::MouseEvent& event) override;
	void mouseEnter(const juce::MouseEvent& event) override;
private:
	CreateColoursConfiguration createColoursConfiguration;
};