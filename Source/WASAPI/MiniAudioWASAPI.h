#pragma once

#include <JuceHeader.h>

#include "miniaudio.h"

#include "../AudioSource/AudioSourceCreatorInterface.h"

#include "../Components/ComponentManagement.h"

#include "../PushSampleIntoJuceAudioBuffer.h"
#include "../CreatePushSampleIntoJuceAudioBufferInstance.h"

class MiniAudioWASAPI : public AudioSourceCreatorInterface
{
public:
	MiniAudioWASAPI();
	~MiniAudioWASAPI();

    void stopDevice();

	void data_callback(ma_device* pDevice, void* pOutput, const void* pInput, ma_uint32 frameCount);

private:
	PushSampleIntoJuceAudioBuffer<float>& pushSampleIntoJuceAudioBuffer = CreatePushSampleIntoJuceAudioBufferInstance::getInstance();
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MiniAudioWASAPI);
};