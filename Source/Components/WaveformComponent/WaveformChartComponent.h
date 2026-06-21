#pragma once

#include <JuceHeader.h>

#include "../../CreateConfiguration/CreateColoursConfiguration.h"
#include "../../GUI/Components/DrawBounds.h"
#include "../../GUI/Components/ComponentHeader.h"
#include "ChartReferenceLine.h"
#include "../../IntermediateDataLayer/AudioLayerManager.h"

enum WaveformChartMode
{
    waveformChartLeft,
    waveformChartRight,
    waveformChartLR,
    waveformChartMerge
};

class WaveformChartComponent : public juce::Component, private juce::ValueTree::Listener, juce::Slider::Listener
{
public:
	WaveformChartComponent();
	~WaveformChartComponent();

    void paint(juce::Graphics& g) override;
    void mouseDown(const juce::MouseEvent& event) override;
    void mouseEnter(const juce::MouseEvent& event) override;
    void mouseExit(const juce::MouseEvent&) override;

    void valueTreePropertyChanged(juce::ValueTree& tree, const juce::Identifier& property) override;
    void sliderValueChanged(juce::Slider* slider);

    void clear();
    void setWaveformChartMode(WaveformChartMode);
    void exchangeLRposition();
    void reversePolarityFunction();
    void openOrCloseTrigger();

    int findTriggerOffset(const std::vector<TruePeak<float>::Peak>* buffer, int& firstPointIndex, int peaksPerBlock, int limitedPeaks, int targetPeriods);

    const std::vector<float> dbValues = {
    -6.f, -12.f
    };

    juce::Rectangle<int> drawArea{ 0,0,500,150 };

    uint16_t callbackId = 0;

    DrawBounds drawBounds;
    ComponentHeader componentHeader{ juce::String("WaveformChart") };
	ChartReferenceLine chartReferenceLine;

    using Callback = std::function<void()>;
    Callback cb = nullptr;

private:
    std::shared_ptr<RMSDataLayer<float>> rmsDataLayer;
    std::shared_ptr<TruePeak<float>> truePeak;

    int startNum = 0;
    int endNum = 0;

    int totalReadSamples = 2000;
    int totalChannels = 2;

    float currentWindow = 0.064f;

    bool exchange = false;
    bool trigger = true;
    float reversePolarity = 1.0f;
    int pointNum = 3;
    float gain = 1.0f;

    static constexpr float SINGLE_BLOCK_DURATION = 0.021333333f;

    WaveformChartMode currentMode = waveformChartLeft;

    juce::ValueTree waveformChartCat;

    juce::Colour lineColorL = juce::Colours::blueviolet;
    juce::Colour fillColorL = juce::Colours::blueviolet;
    juce::Colour gradientColorOfLinesL = juce::Colours::white;

    juce::Colour lineColorR = juce::Colours::blueviolet;
    juce::Colour fillColorR = juce::Colours::blueviolet;
    juce::Colour gradientColorOfLinesR = juce::Colours::white;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(WaveformChartComponent);
};