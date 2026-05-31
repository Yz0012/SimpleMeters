
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

};