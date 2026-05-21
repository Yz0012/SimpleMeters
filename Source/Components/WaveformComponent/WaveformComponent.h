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
        juce::Colour waveformColour = juce::Colours::cyan,
        float lineThickness = 2.0f,
        juce::Colour fillColour = juce::Colours::green);
    void drawAxis(juce::Graphics& g, juce::Rectangle<int> area);

    void renderNextFrame(juce::Graphics& g, juce::Rectangle<int> bounds);

    const juce::AudioBuffer<float> *localAudioBuffer;
    const float* localAudioBufferRMS;

    std::shared_ptr<CircularImageBuffer> imageRingBuffer;

    const std::vector<float> dbValues = {
    -6.f, -12.f
    };

    juce::Rectangle<int> tileArea{ 0,0,8,150 };
    juce::Rectangle<int> drawArea{ 0,0,500,150 };

    uint16_t callbackId = 0;
private:
    CreateColoursConfiguration* createColoursConfiguration;
    juce::Colour lineColor = juce::Colours::blueviolet;
    juce::Colour fillColor = juce::Colours::blueviolet;
    juce::Colour gradientColorOfLines = juce::Colours::white;

	int tileSize = 0;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(WaveformComponent)
};