
#include "Header.h"

Header::Header()
{
	addAndMakeVisible(&windowControl);
	addAndMakeVisible(&WASAPIButton);
	addAndMakeVisible(&windowsSizeButton);
	addAndMakeVisible(&headerFixedButton);
}

Header::~Header()
{
}