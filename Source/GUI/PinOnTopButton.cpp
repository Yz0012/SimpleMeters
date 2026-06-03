
#include "PinOnTopButton.h"

PinOnTopButton::PinOnTopButton()
{
}

PinOnTopButton::~PinOnTopButton()
{
}

void PinOnTopButton::paint(juce::Graphics& g)
{
	if (pinFixed)
	{
		g.drawImageAt(juce::ImageCache::getFromMemory(BinaryData::pinCloseIcon_png, BinaryData::pinCloseIcon_pngSize), 0, 0);
	}
	else
	{
		g.drawImageAt(juce::ImageCache::getFromMemory(BinaryData::pinIcon_png, BinaryData::pinIcon_pngSize), 0, 0);
	}
}

void PinOnTopButton::mouseDown(const juce::MouseEvent& event)
{
	this->callBackFunction();
	pinFixed = !pinFixed;
	repaint();
}


void PinOnTopButton::mouseEnter(const juce::MouseEvent&)
{
	setMouseCursor(juce::MouseCursor::PointingHandCursor);
}

void PinOnTopButton::callBackFunction()
{
	if (onClick == nullptr) return;
	onClick();
}