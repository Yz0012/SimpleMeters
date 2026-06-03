
#include "HeaderFixedButton.h"

HeaderFixedButton::HeaderFixedButton()
{
}

HeaderFixedButton::~HeaderFixedButton()
{
}

void HeaderFixedButton::paint(juce::Graphics& g)
{
	if (headerFixed)
	{
		g.drawImageAt(juce::ImageCache::getFromMemory(BinaryData::closeHeaderPinIcon_png, BinaryData::closeHeaderPinIcon_pngSize), 0, 0);
	}
	else
	{
		g.drawImageAt(juce::ImageCache::getFromMemory(BinaryData::headerPinIcon_png, BinaryData::headerPinIcon_pngSize), 0, 0);
	}
}

void HeaderFixedButton::mouseDown(const juce::MouseEvent& event)
{
	headerFixed = !headerFixed;
	repaint();
}

void HeaderFixedButton::mouseEnter(const juce::MouseEvent&)
{
	setMouseCursor(juce::MouseCursor::PointingHandCursor);
}

bool HeaderFixedButton::getHeaderFixed() const
{
	return headerFixed;
}