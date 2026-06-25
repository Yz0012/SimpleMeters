#include "VectorOscilloscopeComponent.h"

VectorOscilloscopes::VectorOscilloscopes()
{
    CreateColoursConfiguration& createColoursConfiguration = CreateColoursConfiguration::getInstance();

    vectorCat = createColoursConfiguration.getCurrentValueTree()
        .getChildWithProperty("name", "SpectrumAnalyzer");
    addAndMakeVisible(&drawBounds);
    addAndMakeVisible(&componentHeader);
	addAndMakeVisible(&oscilloscopeReferenceLines);

	vectorCat.addListener(this);
}

VectorOscilloscopes::~VectorOscilloscopes()
{
	vectorCat.removeListener(this);
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

void VectorOscilloscopes::mouseDown(const juce::MouseEvent& event)
{
    if (event.mods.isPopupMenu())
    {
        if (cb == nullptr) return;
        cb();
    }
}

void VectorOscilloscopes::mouseEnter(const juce::MouseEvent& event)
{
    drawBounds.setVisible(true);
    componentHeader.setVisible(true);
}

void VectorOscilloscopes::mouseExit(const juce::MouseEvent&)
{
    if (!componentHeader.isMouseOver() && !componentHeader.headerFixedButton.getHeaderFixed())
    {
        drawBounds.setVisible(false);
        componentHeader.setVisible(false);
    }
}

void VectorOscilloscopes::valueTreePropertyChanged(juce::ValueTree& tree, const juce::Identifier& property)
{
    if (property == juce::Identifier("hex"))
    {
        repaint();
    }
}