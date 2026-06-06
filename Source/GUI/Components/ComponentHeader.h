
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
private:

};