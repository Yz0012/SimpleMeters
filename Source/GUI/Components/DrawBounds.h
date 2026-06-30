
#pragma once

#include <JuceHeader.h>

#include "../../CreateConfiguration/CreateColoursConfiguration.h"

class DrawBounds : public juce::Component
{
public:
	DrawBounds();
	~DrawBounds();

	void paint(juce::Graphics&) override;

private:
	juce::Colour boundsColour;
	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(DrawBounds);
};