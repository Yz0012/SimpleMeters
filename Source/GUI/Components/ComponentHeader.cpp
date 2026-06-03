
#include "ComponentHeader.h"

ComponentHeader::ComponentHeader(juce::String identity) : identity(identity)
{
	addAndMakeVisible(&themeConfigButton);
	addAndMakeVisible(&componentControl);
}

ComponentHeader::~ComponentHeader()
{
}