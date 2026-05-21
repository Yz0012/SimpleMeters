
#include "WASAPIButton.h"

WASAPIButton::WASAPIButton()
{
}

WASAPIButton::~WASAPIButton()
{
}

void WASAPIButton::paint(juce::Graphics& g)
{
	if (isOpen)
	{
		g.drawImageAt(juce::ImageCache::getFromMemory(BinaryData::closeWASAPIButton_png, BinaryData::closeWASAPIButton_pngSize), 0, 0);
	}
	else
	{
		g.drawImageAt(juce::ImageCache::getFromMemory(BinaryData::openWASAPIButton_png, BinaryData::openWASAPIButton_pngSize), 0, 0);
	}
}

void WASAPIButton::mouseDown(const juce::MouseEvent& event)
{
	callbackFunction();
}

void WASAPIButton::mouseEnter(const juce::MouseEvent&)
{
	setMouseCursor(juce::MouseCursor::PointingHandCursor);
}

void WASAPIButton::callbackFunction() const
{
	if (onClick) {
		onClick();
	}
}