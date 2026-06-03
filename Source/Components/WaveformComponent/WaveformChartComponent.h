#pragma once

#include <JuceHeader.h>

#include "../../CreateConfiguration/CreateColoursConfiguration.h"
#include "../../GUI/Components/DrawBounds.h"
#include "../../GUI/Components/ComponentHeader.h"

class WaveformChartComponent : public juce::Component, private juce::ValueTree::Listener
{
public:
	WaveformChartComponent();
	~WaveformChartComponent();

    void paint(juce::Graphics& g) override;
    void mouseDown(const juce::MouseEvent& event) override;
    void mouseEnter(const juce::MouseEvent& event) override;
    void mouseExit(const juce::MouseEvent&) override;

    void valueTreePropertyChanged(juce::ValueTree& tree, const juce::Identifier& property) override;

    void clear();

    void pushStereoBuffer(const juce::AudioBuffer<float>* localAudioBuffer, const float localAudioBufferRMS);

    const std::vector<float> dbValues = {
    -6.f, -12.f
    };

    juce::Rectangle<int> drawArea{ 0,0,500,150 };

    uint16_t callbackId = 0;

    DrawBounds drawBounds;
    ComponentHeader componentHeader{ juce::String("WaveformChart") };

    using Callback = std::function<void()>;
    Callback cb = nullptr;
private:
    juce::ValueTree waveformChartCat;

    const juce::AudioBuffer<float>* localAudioBuffer = nullptr;
    //消除拷贝
    float localAudioBufferRMS;

    juce::Colour lineColorL = juce::Colours::blueviolet;
    juce::Colour fillColorL = juce::Colours::blueviolet;
    juce::Colour gradientColorOfLinesL = juce::Colours::white;

    juce::Colour lineColorR = juce::Colours::blueviolet;
    juce::Colour fillColorR = juce::Colours::blueviolet;
    juce::Colour gradientColorOfLinesR = juce::Colours::white;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(WaveformChartComponent);
};