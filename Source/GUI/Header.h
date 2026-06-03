
#pragma once

#include <JuceHeader.h>

#include "WindowControl.h"
#include "WASAPIButton.h"
#include "WindowsSizeButton.h"
#include "HeaderFixedButton.h"
#include "ThemeConfigurationButton.h"
#include "PinOnTopButton.h"

class Header : public juce::Component , public juce::SettableTooltipClient
{
public:
	Header();
	~Header();

	WindowControl windowControl;
	WASAPIButton WASAPIButton;
	WindowsSizeButton windowsSizeButton;
	HeaderFixedButton headerFixedButton;
	ThemeConfigurationButton themeConfigurationButton;
	PinOnTopButton pinOnTopButton;

private:
	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(Header);
};