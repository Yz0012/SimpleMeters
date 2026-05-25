
#pragma once
#include <JuceHeader.h>

#include "../../CreateConfiguration/CreateColoursConfiguration.h"
#include "../../GUI/Components/ComponentControl.h"

enum AnalysisMode
{
    Mono,
    Stereo,
    LR,
    Interleaved
};

class SpectrumAnalyser : public juce::Component , juce::Timer
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
    void drawFrame(juce::Graphics& g,juce::Rectangle<int> bounds);
    void drawSingleCurve(juce::Graphics& g,
        juce::Rectangle<int> bounds,
        const float* scopeData,
        float* scopeDataStorage,
        float* gapSmoothedScopeData,
        juce::Colour lineColour,
        juce::Colour fillColour);
    void drawFrequencyAxis(juce::Graphics& g,juce::Rectangle<int> bounds);
    void setAnalysisMode(AnalysisMode mode);

    void paint(juce::Graphics& g) override;
	void timerCallback() override;
    void mouseEnter(const juce::MouseEvent& event) override;
    void mouseExit(const juce::MouseEvent&) override;

    void callStopTimer();
    void callstartTimerHz(int hz);
    void checkProcessBufferActivity();
    void scopeDataReset();

    juce::Rectangle<int> drawArea{ 0,0,500,150 };

    uint16_t callbackId = 0;
    uint16_t callbackIdM = 0;
    uint16_t callbackIdS = 0;

    ComponentControl componentControl;
private:
    double lastProcessTime = 0;

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
    float gapSmoothedScopeData[scopeSize];
    float scopeDataStorage[scopeSize];

    float fifo2[fftSize];
    float fftData2[fftSize * 2];
    int fifoIndex2 = 0;
    bool nextFFTBlockReady2 = false;

    float scopeData2[scopeSize];
    float scopeDataStorage2[scopeSize];
    float gapSmoothedScopeData2[scopeSize];


    float level = 0.0f;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SpectrumAnalyser);
};