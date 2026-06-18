#pragma once
#include <JuceHeader.h>
#include "../../CreateConfiguration/CreateColoursConfiguration.h"
#include "../../GUI/Components/DrawBounds.h"
#include "../../GUI/Components/ComponentHeader.h"
#include "WaveformComponentReferenceLine.h"
#include "../../IntermediateDataLayer/AudioLayerManager.h"

enum WaveformMode
{
    waveformLeft,
    waveformRight,
    waveformLR,
    waveformMerge,
    waveformSeparate
};

class WaveformComponent : public juce::Component, private juce::ValueTree::Listener, juce::Slider::Listener
{
public:

    WaveformComponent();
    ~WaveformComponent();

    void setTimeInterval(float seconds);
    void setWaveformMode(WaveformMode);
    void exchangeLRposition();
    void paint(juce::Graphics& g) override;
    void resized() override;
    void valueTreePropertyChanged(juce::ValueTree& tree, const juce::Identifier& property);
    void mouseDown(const juce::MouseEvent& event) override;
    void mouseEnter(const juce::MouseEvent& event) override;
    void mouseExit(const juce::MouseEvent&) override;
    void sliderValueChanged(juce::Slider* slider);

    DrawBounds drawBounds;
    ComponentHeader componentHeader{ juce::String("Waveform") };
    WaveformComponentReferenceLine waveformReferenceLine;

    uint16_t callbackId = 0;

    using Callback = std::function<void()>;
    Callback cb = nullptr;

    juce::Rectangle<float> drawArea;
private:
    juce::ValueTree waveformCat;
    std::shared_ptr<RMSDataLayer<float>> rmsDataLayer;
    std::shared_ptr<TruePeak<float>> truePeak;

    juce::Colour lineColorL = juce::Colours::blueviolet;
    juce::Colour gradientColorOfLinesL = juce::Colours::white;

    juce::Colour lineColorR = juce::Colours::blueviolet;
    juce::Colour gradientColorOfLinesR = juce::Colours::white;

    WaveformMode currentMode = waveformMerge;

    float currentWindow = 0.5f;

    int64_t totalBlocksReceived = 0;

    static constexpr float SINGLE_BLOCK_DURATION = 0.021333333f;

    bool exchange = false;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(WaveformComponent);
};