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
        int lowIndex;
        float lowValue;
        int midIndex;
        float midValue;
        int highIndex;
        float highValue;
        float value;
        float index;
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

        for (int i = 0; i < scopeSize; ++i)
        {
            scopeData[i] = mindB;
        }
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
                    auto skewedProportionX = j / (float)scopeSize;
                    auto fftDataIndex = juce::jlimit(0, fftSize / 2, (int)(skewedProportionX * (float)fftSize * 0.5f));
                    auto sourceDecibels = juce::Decibels::gainToDecibels(fftData[fftDataIndex]) - juce::Decibels::gainToDecibels((float)fftSize);
                    scopeData[j] = sourceDecibels;
                }
                extractPeaks();

                if (onFftReady) onFftReady();
                fifoIndex = 0;
            }
        }
    }

    void extractPeaks()
    {
        absoluteMaxPeak = { 0, mindB, 0, mindB, 0, mindB, mindB , 0.0f };

        for (int i = 0; i < scopeSize / 16; i++)
        {
            if (absoluteMaxPeak.lowValue < scopeData[i])
            {
                absoluteMaxPeak.lowIndex = i;
                absoluteMaxPeak.lowValue = scopeData[i];
                if (absoluteMaxPeak.value < scopeData[i])
                {
                    absoluteMaxPeak.value = scopeData[i];
                    absoluteMaxPeak.index = i;
                }
            }
        }
        for (int i = scopeSize / 16; i < scopeSize / 2; i++)
        {
            if (absoluteMaxPeak.midValue < scopeData[i])
            {
                absoluteMaxPeak.midIndex = i;
                absoluteMaxPeak.midValue = scopeData[i];
                if (absoluteMaxPeak.value < scopeData[i])
                {
                    absoluteMaxPeak.value = scopeData[i];
                    absoluteMaxPeak.index = i;
                }
            }
        }
        for (int i = scopeSize / 2; i < scopeSize; i++)
        {
            if (absoluteMaxPeak.highValue < scopeData[i])
            {
                absoluteMaxPeak.highIndex = i;
                absoluteMaxPeak.highValue = scopeData[i];
                if (absoluteMaxPeak.value < scopeData[i])
                {
                    absoluteMaxPeak.value = scopeData[i];
                    absoluteMaxPeak.index = i;
                }
            }
        }

    }

    T extractInterpolationFreq(int index)
    {
        const float sampleRate = 48000.0f;

        float refinedBinIdx = static_cast<T>(index);

        if (index > 0 && index < scopeSize - 1)
        {
            float y1 = scopeData[index - 1];
            float y2 = scopeData[index];
            float y3 = scopeData[index + 1];

            float denominator = 2.0f * (2.0f * y2 - y1 - y3);
            if (std::abs(denominator) > 1e-5f)
            {
                float delta = (y3 - y1) / denominator;

                delta = juce::jlimit(-0.5f, 0.5f, delta);

                refinedBinIdx += delta;
            }
        }

        return (refinedBinIdx * (sampleRate * 0.5f)) / static_cast<float>(scopeSize);
    }

    void setOnFftReadyCallback(std::function<void()> callback) { onFftReady = std::move(callback); }

    const std::array<float, scopeSize>& getScopeData() const { return scopeData; }
    PeakPoint getAbsoluteMaxPeak() const { return absoluteMaxPeak; }

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