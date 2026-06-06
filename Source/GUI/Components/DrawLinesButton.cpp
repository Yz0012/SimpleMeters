
#include "DrawLinesButton.h"

DrawLinesButton::DrawLinesButton()
{
}

DrawLinesButton::~DrawLinesButton()
{
}

void DrawLinesButton::paint(juce::Graphics& g)
{
    if (isOpen)
    {
        g.drawImageAt(juce::ImageCache::getFromMemory(BinaryData::closeDrawReferenceLine_png, BinaryData::closeDrawReferenceLine_pngSize), 0, 0);
    }
    else
    {
        g.drawImageAt(juce::ImageCache::getFromMemory(BinaryData::DrawReferenceLine_png, BinaryData::DrawReferenceLine_pngSize), 0, 0);
    }
}

void DrawLinesButton::mouseDown(const juce::MouseEvent& event)
{
    callbackFunction();
}

void DrawLinesButton::mouseEnter(const juce::MouseEvent& event)
{
    setMouseCursor(juce::MouseCursor::PointingHandCursor);
}

void DrawLinesButton::callbackFunction() const
{
    if (onClick) {
        onClick();
    }
}