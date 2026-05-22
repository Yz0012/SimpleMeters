
#pragma once
#include <JuceHeader.h>

#include "../../CreateConfiguration/CreateColoursConfiguration.h"

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

    void pushNextSampleIntoFifo(float sample) noexcept;
    void drawNextFrameOfSpectrum();
    void drawFrame(juce::Graphics& g,juce::Rectangle<int> bounds);
    void drawFrequencyAxis(juce::Graphics& g,juce::Rectangle<int> bounds);

    void paint(juce::Graphics& g) override;
	void timerCallback() override;

    void callStopTimer();
    void callstartTimerHz(int hz);

    juce::Rectangle<int> drawArea{ 0,0,500,150 };

    uint16_t callbackId = 0;
    uint16_t callbackIdM = 0;
    uint16_t callbackIdS = 0;
private:
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

    float level = 0.0f;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SpectrumAnalyser);
};