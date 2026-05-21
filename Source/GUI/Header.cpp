
#include "Header.h"

Header::Header()
{
	addAndMakeVisible(&windowControl);
	addAndMakeVisible(&WASAPIButton);
	addAndMakeVisible(&windowsSizeButton);
}

Header::~Header()
{
}