
#pragma once

#include <JuceHeader.h>

#include "ComponentThemeConfigurationButton.h"
#include "ComponentControl.h"
#include "DrawLinesButton.h"
#include "../HeaderFixedButton.h"

class ComponentHeader : public juce::Component
{
public:
	ComponentHeader(juce::String);
	~ComponentHeader();

	juce::String identity;

	ComponentThemeConfigurationButton themeConfigButton{ identity };

	ComponentControl componentControl;
	HeaderFixedButton headerFixedButton;
	DrawLinesButton drawLinesButton;
	juce::Slider knob{ juce::Slider::RotaryVerticalDrag, juce::Slider::TextBoxBelow };
	juce::Slider knobTwo{ juce::Slider::RotaryVerticalDrag, juce::Slider::TextBoxBelow };
private:
	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ComponentHeader);
};