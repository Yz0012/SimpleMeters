
#include "Header.h"

Header::Header()
{
	addAndMakeVisible(&windowControl);
	addAndMakeVisible(&WASAPIButton);
	addAndMakeVisible(&windowsSizeButton);
	addAndMakeVisible(&headerFixedButton);
	addAndMakeVisible(&themeConfigurationButton);
	addAndMakeVisible(&pinOnTopButton);
}

Header::~Header()
{
}