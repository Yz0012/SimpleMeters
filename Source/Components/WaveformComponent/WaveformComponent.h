#pragma once
#include <JuceHeader.h>

#include "../Source/CircularImageBuffer.h"
#include "../../CreateConfiguration/CreateColoursConfiguration.h"

class WaveformComponent : public juce::Component
{
public:
    WaveformComponent();
    ~WaveformComponent();

    void paint(juce::Graphics& g) override;

    void clear();

    void drawWaveform(
        const juce::AudioBuffer<float>& localAudioBuffer,
        const float localAudioBufferRMS);
    void drawAxis(juce::Graphics& g, juce::Rectangle<int> area);

    void renderNextFrame(juce::Graphics& g, juce::Rectangle<int> bounds);

    std::shared_ptr<CircularImageBuffer> imageRingBuffer;
    std::weak_ptr<CircularImageBuffer> weakBuffer;

    const std::vector<float> dbValues = {
    -6.f, -12.f
    };

    juce::Rectangle<int> tileArea{ 0,0,8,150 };
    juce::Rectangle<int> drawArea{ 0,0,500,150 };

    uint16_t callbackId = 0;
private:
    juce::Colour lineColorL = juce::Colours::blueviolet;
    juce::Colour fillColorL = juce::Colours::blueviolet;
    juce::Colour gradientColorOfLinesL = juce::Colours::white;

    juce::Colour lineColorR = juce::Colours::blueviolet;
    juce::Colour fillColorR = juce::Colours::blueviolet;
    juce::Colour gradientColorOfLinesR = juce::Colours::white;

	int tileSize = 0;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(WaveformComponent)
};