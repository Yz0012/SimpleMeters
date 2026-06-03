
#pragma once

#include <JuceHeader.h>

#include "ComponentThemeConfigurationButton.h"
#include "ComponentControl.h"

class ComponentHeader : public juce::Component
{
public:
	ComponentHeader(juce::String);
	~ComponentHeader();

	juce::String identity;

	ComponentThemeConfigurationButton themeConfigButton{ identity };

	ComponentControl componentControl;
private:

};