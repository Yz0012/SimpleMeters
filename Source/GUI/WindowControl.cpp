
#include "WindowControl.h"

WindowControl::WindowControl()
{
}

WindowControl::~WindowControl()
{
}

void WindowControl::paint(juce::Graphics& g)
{
	g.drawImageAt(juce::ImageCache::getFromMemory(BinaryData::closeIcon_png, BinaryData::closeIcon_pngSize), 0, 0);
}

void WindowControl::userTriedToCloseWindow()
{
	juce::JUCEApplication::getInstance()->systemRequestedQuit();
}

void WindowControl::mouseDown(const juce::MouseEvent& event)
{
	userTriedToCloseWindow();
}

void WindowControl::mouseEnter(const juce::MouseEvent&)
{
	setMouseCursor(juce::MouseCursor::PointingHandCursor);
}