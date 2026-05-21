
#include "BufferSizeButton.h"

BufferSizeButton::BufferSizeButton()
{
}

BufferSizeButton::~BufferSizeButton()
{
}

void BufferSizeButton::paint(juce::Graphics& g)
{
		g.drawImageAt(juce::ImageCache::getFromMemory(BinaryData::closeWASAPIButton_png, BinaryData::closeWASAPIButton_pngSize), 0, 0);
}

void BufferSizeButton::mouseEnter(const juce::MouseEvent&)
{
	setMouseCursor(juce::MouseCursor::PointingHandCursor);
}