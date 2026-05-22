#define MINIAUDIO_IMPLEMENTATION
#include "MiniAudioWASAPI.h"

static void staticCallback(ma_device* pDevice, void* pOutput, const void* pInput, ma_uint32 frameCount) {
    MiniAudioWASAPI* self = static_cast<MiniAudioWASAPI*>(pDevice->pUserData);
    if (self) {
        self->data_callback(pDevice, pOutput, pInput, frameCount);
    }
}

ma_device device;

MiniAudioWASAPI::MiniAudioWASAPI()
{
    ma_device_config deviceConfig = ma_device_config_init(ma_device_type_loopback);
    deviceConfig.capture.pDeviceID = NULL;
    deviceConfig.capture.format = ma_format_f32;
    deviceConfig.capture.channels = 2;
    deviceConfig.sampleRate = 48000;
    deviceConfig.dataCallback = staticCallback;
    deviceConfig.pUserData = this;

    ma_device_init(NULL, &deviceConfig, &device);
    ma_device_start(&device);

    DBG("WASAPI device started");
}

MiniAudioWASAPI::~MiniAudioWASAPI()
{
	ma_device_uninit(&device);
	DBG("WASAPI device uninit");
}

void MiniAudioWASAPI::data_callback(ma_device* pDevice, void* pOutput, const void* pInput, ma_uint32 frameCount)
{
    if (pInput == nullptr)
        return;

    const float* inputSamples = static_cast<const float*>(pInput);

    ma_uint32 channels = pDevice->capture.channels;

    for (ma_uint32 frame = 0; frame < frameCount; ++frame)
    {
		pushSampleIntoJuceAudioBuffer.pushSample(0, inputSamples[frame * channels + 0]);
		pushSampleIntoJuceAudioBuffer.pushSample(1, inputSamples[frame * channels + 1]);
    }
}

void MiniAudioWASAPI::stopDevice()
{
    ma_device_stop(&device);
}