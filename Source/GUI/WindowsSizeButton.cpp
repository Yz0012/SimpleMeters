
#include "WindowsSizeButton.h"

WindowsSizeButton::WindowsSizeButton()
{
}

WindowsSizeButton::~WindowsSizeButton()
{
}

void WindowsSizeButton::paint(juce::Graphics& g)
{
	g.drawImageAt(juce::ImageCache::getFromMemory(BinaryData::resizeWindowIcon_png,BinaryData::resizeWindowIcon_pngSize),0,0);
}

void WindowsSizeButton::mouseDown(const juce::MouseEvent& event)
{
    callbackFunction();
}

void WindowsSizeButton::mouseEnter(const juce::MouseEvent& event)
{
	setMouseCursor(juce::MouseCursor::PointingHandCursor);
}

void WindowsSizeButton::callbackFunction() const
{
    if (onClick) {
        onClick();
    }
}