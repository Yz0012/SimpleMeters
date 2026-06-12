
#pragma once

#include "PushSampleIntoJuceAudioBuffer.h"

class CreatePushSampleIntoJuceAudioBufferInstance
{
public:
	static PushSampleIntoJuceAudioBuffer<float>& getInstance()
	{
		static PushSampleIntoJuceAudioBuffer<float> instance(2,1024);
		return instance;
	}

private:
	CreatePushSampleIntoJuceAudioBufferInstance() = default;
	~CreatePushSampleIntoJuceAudioBufferInstance() = default;
};
