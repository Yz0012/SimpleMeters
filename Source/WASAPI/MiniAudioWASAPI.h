#pragma once

#include <JuceHeader.h>

#include "miniaudio.h"

#include "../AudioSource/AudioSourceCreatorInterface.h"

#include "../Components/VectorOscilloscopeComponent/VectorOscilloscopeComponent.h"
#include "../Components/WaveformComponent/WaveformComponent.h"
#include "../Components/Spectrum/SpectrumAnalyser.h"

#include "../PushSampleIntoJuceAudioBuffer.h"

class MiniAudioWASAPI : public AudioSourceCreatorInterface
{
public:
	MiniAudioWASAPI();
	~MiniAudioWASAPI();

    void stopDevice();

    SpectrumAnalyser* getSpectrumAnalyser();
    WaveformComponent* getWaveformComponent();

	void data_callback(ma_device* pDevice, void* pOutput, const void* pInput, ma_uint32 frameCount);

    std::unique_ptr<PushSampleIntoJuceAudioBuffer<float>> pushSampleIntoJuceAudioBuffer;

private:

    std::unique_ptr<WaveformComponent> waveformComponent;
    std::unique_ptr<SpectrumAnalyser> spectrumAnalyser;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MiniAudioWASAPI);
};