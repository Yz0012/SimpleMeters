
#pragma once

#include "WindowControl.h"
#include "WASAPIButton.h"

class Header : public juce::Component , public juce::SettableTooltipClient
{
public:
	Header();
	~Header();

	WindowControl windowControl;
	WASAPIButton WASAPIButton;

private:
	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(Header);
};