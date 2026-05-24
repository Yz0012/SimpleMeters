#pragma once

#include <JuceHeader.h>

#include "../../CreateConfiguration/CreateColoursConfiguration.h"

class WaveformChartComponent : public juce::Component
{
public:
	WaveformChartComponent();
	~WaveformChartComponent();

    void paint(juce::Graphics& g) override;

    void clear();

    void pushStereoBuffer(const juce::AudioBuffer<float>* localAudioBuffer, const float* localAudioBufferRMS);

    const std::vector<float> dbValues = {
    -6.f, -12.f
    };

    juce::Rectangle<int> drawArea{ 0,0,500,150 };

    uint16_t callbackId = 0;
private:
    const juce::AudioBuffer<float>* localAudioBuffer = nullptr;
    const float* localAudioBufferRMS = nullptr;

    juce::Colour lineColorL = juce::Colours::blueviolet;
    juce::Colour fillColorL = juce::Colours::blueviolet;
    juce::Colour gradientColorOfLinesL = juce::Colours::white;

    juce::Colour lineColorR = juce::Colours::blueviolet;
    juce::Colour fillColorR = juce::Colours::blueviolet;
    juce::Colour gradientColorOfLinesR = juce::Colours::white;
};