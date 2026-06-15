
#pragma once

#include <JuceHeader.h>

#include "../../CreateConfiguration/CreateColoursConfiguration.h"
#include "../../GUI/Components/DrawBounds.h"
#include "../../GUI/Components/ComponentHeader.h"
#include "EQReferenceLineComponent.h"
#include "../../IntermediateDataLayer/AudioLayerManager.h"

class SpectrumAnalyserMono : public juce::Component, juce::Timer, private juce::ValueTree::Listener
{
public:
    SpectrumAnalyserMono();
    ~SpectrumAnalyserMono();

    enum {
        fftOrder = 12,
        fftSize = 1 << fftOrder,
        scopeSize = 1024
    };

    float mindB = -90.0f;
    float maxdB = -0.0f;

    float scopeSizeTransformed = std::log((float)scopeSize + std::exp(1.0f)) - 1;

    void activityCheck();
    void drawFrame(juce::Graphics& g, juce::Rectangle<int> bounds);
    void drawSingleCurve(juce::Graphics& g,
        juce::Rectangle<int> bounds,
        const std::array<float, scopeSize>& scopeData,
        std::array<float, scopeSize>& scopeDataStorage,
        std::array<float, scopeSize>& destScope,
        juce::Colour lineColour,
        juce::Colour fillColour);

    void paint(juce::Graphics& g) override;
    void timerCallback() override;
    void mouseDown(const juce::MouseEvent& event) override;
    void mouseEnter(const juce::MouseEvent& event) override;
    void mouseExit(const juce::MouseEvent&) override;

    void valueTreePropertyChanged(juce::ValueTree& tree, const juce::Identifier& property) override;

    void callStopTimer();
    void callstartTimerHz(int hz);
    void checkProcessBufferActivity();
    void scopeDataReset();

    juce::Rectangle<int> drawArea{ 0,0,500,150 };

    uint16_t callbackId = 0;
    uint16_t callbackIdM = 0;
    uint16_t callbackIdS = 0;

    DrawBounds drawBounds;
    ComponentHeader componentHeader{ juce::String("SpectrumAnalyzerMono") };
    EQReferenceLineComponent eqReferenceLines;

    using Callback = std::function<void()>;
    Callback cb = nullptr;

    std::shared_ptr<FftDataLayer<float>> fftLayer;
private:

    double lastProcessTime = 0;

    juce::ValueTree spectrumCat;

    juce::Colour lineColor = juce::Colour(0xFF8400FF);
    juce::Colour fillColor = juce::Colour(0xFF8400FF);

    const std::array<float, scopeSize>& scopeData;
    std::array<float, scopeSize> scopeDataStorage;

    std::array<float, scopeSize> destScope1;

    float level = 0.0f;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SpectrumAnalyserMono);
};