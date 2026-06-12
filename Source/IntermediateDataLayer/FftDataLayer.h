#pragma once
#include <JuceHeader.h>
#include "../CreatePushSampleIntoJuceAudioBufferInstance.h"

template <typename T>
class FftDataLayer
{
public:
    enum {
        fftOrder = 12,
        fftSize = 1 << fftOrder,
        scopeSize = 1024
    };

    struct PeakPoint {
        int midIndex;
        float value;
    };

    FftDataLayer()
        : forwardFFT(fftOrder),
          window(fftSize, juce::dsp::WindowingFunction<float>::hann)
    {
        std::fill(fifo.begin(), fifo.end(), 0.0f);
        std::fill(fftData.begin(), fftData.end(), 0.0f);
        std::fill(scopeData.begin(), scopeData.end(), 0.0f);

        auto& instance = CreatePushSampleIntoJuceAudioBufferInstance::getInstance();
        callBackId = instance.add(
            [this, &instance]() {
                this->processAudioBuffer(instance.getLocalAudioBufferReference());
            }
        );
    }

    ~FftDataLayer()
    {
        CreatePushSampleIntoJuceAudioBufferInstance::getInstance().remove(callBackId);
    }

    void processAudioBuffer(const juce::AudioBuffer<T>& buffer)
    {
        const int numSamples = buffer.getNumSamples();
        const int numChannels = buffer.getNumChannels();
        const T* leftChannel = buffer.getReadPointer(0);
        const T* rightChannel = (numChannels > 1) ? buffer.getReadPointer(1) : leftChannel;

        for (int i = 0; i < numSamples; ++i)
        {
            fifo[fifoIndex++] = static_cast<float>((leftChannel[i] + rightChannel[i]) / 2.0f);

            if (fifoIndex == fftSize)
            {
                std::copy(fifo.begin(), fifo.end(), fftData.begin());

                window.multiplyWithWindowingTable(fftData.data(), fftSize);
                forwardFFT.performFrequencyOnlyForwardTransform(fftData.data());

                for (int j = 0; j < scopeSize; ++j)
                {
                    int bin = j * (fftSize / 2) / scopeSize;
                    float level = fftData[bin] / (float)fftSize; 
                    float decibels = juce::Decibels::gainToDecibels(level, mindB);
                    scopeData[j] = decibels;
                }

                if (onFftReady) onFftReady();
                fifoIndex = 0;
            }
        }
    }

    void extractPeaks()
    {
        absoluteMaxPeak = { 0, 0.0f };

        for (int i = 0; i < scopeSize; i++)
        {
            if (absoluteMaxPeak.value < scopeData[i])
            {
                absoluteMaxPeak.midIndex = i;
                absoluteMaxPeak.value = scopeData[i];
            }
        }
    }

    void setOnFftReadyCallback(std::function<void()> callback) { onFftReady = std::move(callback); }

    std::array<float, scopeSize> getScopeData() const { return scopeData; }
    PeakPoint getAbsoluteMaxPeak() const { return absoluteMaxPeak; }

    void copyScopeDataTo(float* destination, int maxElements) const
    {
        const int elementsToCopy = std::min(static_cast<int>(scopeSize), maxElements);

        std::copy(scopeData.begin(), scopeData.begin() + elementsToCopy, destination);
    }

    uint16_t callBackId;

private:
    juce::dsp::FFT forwardFFT;
    juce::dsp::WindowingFunction<float> window;
    std::array<float, fftSize * 2> fifo;
    std::array<float, fftSize * 2> fftData;
    std::array<float, scopeSize> scopeData;

    PeakPoint absoluteMaxPeak;

    int fifoIndex = 0;
    float mindB = -90.0f;
    float maxdB = -0.0f;

    std::function<void()> onFftReady;
};