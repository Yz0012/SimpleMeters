#include "VectorOscilloscopeComponent.h"

VectorOscilloscopes::VectorOscilloscopes()
{
    addAndMakeVisible(&componentControl);
    addAndMakeVisible(&drawBounds);
}

VectorOscilloscopes::~VectorOscilloscopes()
{
}

void VectorOscilloscopes::pushStereoBuffer(const juce::AudioBuffer<float>* localAudioBuffer)
{
    stereoBuffer = localAudioBuffer;
}

void VectorOscilloscopes::paint(juce::Graphics& g)
{
    if (stereoBuffer == nullptr) return;
    const int numSamples = stereoBuffer->getNumSamples();
    if (numSamples == 0) return;

    const int w = getWidth();
    const int h = getHeight();

    juce::Image img(juce::Image::ARGB, w, h, true);

    {
        juce::Image::BitmapData bitmap(img, juce::Image::BitmapData::writeOnly);
        auto* leftSamples = stereoBuffer->getReadPointer(0);
        auto* rightSamples = stereoBuffer->getReadPointer(1);

        for (int i = 0; i < numSamples; ++i)
        {
            const float mid = (leftSamples[i] + rightSamples[i]) * 0.5f;
            const float side = (leftSamples[i] - rightSamples[i]) * 0.5f;

            int px = (int)((side + 1.0f) * w * 0.5f);
            int py = (int)((mid + 1.0f) * h * 0.5f);

            if (px >= 0 && px < w && py >= 0 && py < h)
            {
                float hue = static_cast<float>(i) / static_cast<float>(numSamples);
                juce::Colour colour = juce::Colour::fromHSV(hue, 1.0f, 1.0f, 1.0f);
                bitmap.setPixelColour(px, py, colour);
            }
        }
    }

    g.drawImageAt(img, 0, 0);
}

void VectorOscilloscopes::mouseEnter(const juce::MouseEvent& event)
{
    componentControl.setVisible(true);
    drawBounds.setVisible(true);
}

void VectorOscilloscopes::mouseExit(const juce::MouseEvent&)
{
    if (!componentControl.isMouseOver())
    {
        componentControl.setVisible(false);
        drawBounds.setVisible(false);
    }
}