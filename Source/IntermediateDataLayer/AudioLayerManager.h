#pragma once
#include "TruePeak.h"
#include "FftDataLayer.h"
#include "RMSDataLayer.h"

template <typename T>
class AudioLayerManager
{
public:
    static AudioLayerManager& getInstance()
    {
        static AudioLayerManager instance;
        return instance;
    }

    std::shared_ptr<TruePeak<T>> getTruePeak()
    {
        std::shared_ptr<TruePeak<T>> ptr = truePeakWeak.lock();
        if (!ptr) {
            ptr = std::make_shared<TruePeak<T>>();
            truePeakWeak = ptr;
        }
        return ptr;
    }

    std::shared_ptr<FftDataLayer<T>> getFftDataLayer()
    {
        std::shared_ptr<FftDataLayer<T>> ptr = fftDataLayerWeak.lock();
        if (!ptr) {
            ptr = std::make_shared<FftDataLayer<T>>();
            fftDataLayerWeak = ptr;
        }
        return ptr;
    }

    std::shared_ptr<RMSDataLayer<T>> getRMSDataLayer()
    {
        std::shared_ptr<RMSDataLayer<T>> ptr = rmsDataLayerWeak.lock();
        if (!ptr)
        {
            ptr = std::make_shared<RMSDataLayer<T>>();
            rmsDataLayerWeak = ptr;
        }
        return ptr;
    }

private:
    AudioLayerManager() = default;
    ~AudioLayerManager() = default;

    AudioLayerManager(const AudioLayerManager&) = delete;
    AudioLayerManager& operator=(const AudioLayerManager&) = delete;

    std::weak_ptr<TruePeak<T>> truePeakWeak;
    std::weak_ptr<FftDataLayer<T>> fftDataLayerWeak;
    std::weak_ptr<RMSDataLayer<T>> rmsDataLayerWeak;
};