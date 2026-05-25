#include "ComponentControl.h"


ComponentControl::ComponentControl()
{
}

ComponentControl::~ComponentControl()
{
}

void ComponentControl::paint(juce::Graphics& g)
{
	g.drawImageAt(juce::ImageCache::getFromMemory(BinaryData::closeIcon10px_png, BinaryData::closeIcon10px_pngSize), 0, 0);
}

void ComponentControl::mouseDown(const juce::MouseEvent& event)
{
	callBackFunction();
}

void ComponentControl::mouseEnter(const juce::MouseEvent&)
{
	setMouseCursor(juce::MouseCursor::PointingHandCursor);
}

void ComponentControl::setCallBackFuntion(Callback cb)
{
	this->cb = cb;
}

void ComponentControl::callBackFunction()
{
	if (cb == nullptr) return;
	cb();
}