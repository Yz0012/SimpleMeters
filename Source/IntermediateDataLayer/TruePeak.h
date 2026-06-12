#pragma once
#include <JuceHeader.h>
#include "../CreatePushSampleIntoJuceAudioBufferInstance.h"

template <typename T>
class TruePeak
{
public:
    TruePeak()
    {
        auto& instance = CreatePushSampleIntoJuceAudioBufferInstance::getInstance();
        callBackId = instance.add(
            [this, &instance]() {
                this->calculateSamplePeak(instance.getLocalAudioBufferReference());
            }
        );
    }

    ~TruePeak()
    {
        CreatePushSampleIntoJuceAudioBufferInstance::getInstance().remove(callBackId);
    }

    void calculateSamplePeak(const juce::AudioBuffer<T>& buffer)
    {
        leftTruePeak = buffer.getMagnitude(0, 0, buffer.getNumSamples());

        if (buffer.getNumChannels() > 1)
            rightTruePeak = buffer.getMagnitude(1, 0, buffer.getNumSamples());
        else
            rightTruePeak = leftTruePeak;
    }

    T getLeftTruePeak() const { return leftTruePeak; }
    T getRightTruePeak() const { return rightTruePeak; }

    uint16_t callBackId;

private:
    T leftTruePeak = 0;
    T rightTruePeak = 0;
};