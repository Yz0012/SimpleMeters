
#pragma once

#include <JuceHeader.h>

#include "../../CreateConfiguration/CreateColoursConfiguration.h"
#include "../../GUI/Components/DrawBounds.h"
#include "../../GUI/Components/ComponentHeader.h"
#include "EQReferenceLineComponent.h"

enum AnalysisMode
{
    Left,
    Right,
    Stereo,
    LR,
    Interleaved
};

class SpectrumAnalyser : public juce::Component, juce::Timer, private juce::ValueTree::Listener
{
public:
    SpectrumAnalyser();
    ~SpectrumAnalyser();

    enum {
        fftOrder = 12,
        fftSize = 1 << fftOrder,
        scopeSize = 1024
    };

    float mindB = -90.0f;
    float maxdB = -0.0f;

    float scopeSizeTransformed = std::log((float)scopeSize + std::exp(1.0f)) - 1;

    void processAudioBuffer(const juce::AudioBuffer<float>& buffer);
    void pushNextSampleIntoFifo(float sample, int channelIndex) noexcept;
    void drawNextFrameOfSpectrum(int channelIndex);
    void drawFrame(juce::Graphics& g, juce::Rectangle<int> bounds);
    void drawSingleCurve(juce::Graphics& g,
        juce::Rectangle<int> bounds,
        const float* scopeData,
        float* scopeDataStorage,
        juce::Colour lineColour,
        juce::Colour fillColour);
    void setAnalysisMode(AnalysisMode mode);

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
    ComponentHeader componentHeader{ juce::String("SpectrumAnalyzer") };
    EQReferenceLineComponent eqReferenceLines;

    using Callback = std::function<void()>;
    Callback cb = nullptr;
private:
    double lastProcessTime = 0;

    juce::ValueTree spectrumCat;

    AnalysisMode currentMode = Interleaved;

    juce::Colour lineColor = juce::Colour(0xFF8400FF);
    juce::Colour fillColor = juce::Colour(0xFF8400FF);

    juce::dsp::FFT forwardFFT;
    juce::dsp::WindowingFunction<float> window;

    float fifo[fftSize];
    float fftData[2 * fftSize];
    int fifoIndex = 0;
    bool nextFFTBlockReady = false;

    float scopeData[scopeSize];
    float scopeDataStorage[scopeSize];

    float fifo2[fftSize];
    float fftData2[fftSize * 2];
    int fifoIndex2 = 0;
    bool nextFFTBlockReady2 = false;

    float scopeData2[scopeSize];
    float scopeDataStorage2[scopeSize];

    float level = 0.0f;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SpectrumAnalyser);
};