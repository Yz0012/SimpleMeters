
#pragma once

#include "WindowControl.h"
#include "WASAPIButton.h"
#include "WindowsSizeButton.h"
#include "HeaderFixedButton.h"

class Header : public juce::Component , public juce::SettableTooltipClient
{
public:
	Header();
	~Header();

	WindowControl windowControl;
	WASAPIButton WASAPIButton;
	WindowsSizeButton windowsSizeButton;
	HeaderFixedButton headerFixedButton;

private:
	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(Header);
};