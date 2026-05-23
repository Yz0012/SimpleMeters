
#pragma once
#include <JuceHeader.h>

template <typename T>
class PushSampleIntoJuceAudioBuffer
{
public:
	PushSampleIntoJuceAudioBuffer(int allChannels, int bufferSize);
	~PushSampleIntoJuceAudioBuffer();

    using Callback = std::function<void()>;
    using CallbackId = uint16_t;

    void pushSample(int channel, T sample)
    {
        T sampleLimited = juce::jlimit(-1.0f, 1.0f, sample);
        audioBuffer.setSample(channel, writeIndexes[channel], sampleLimited);
        ++writeIndexes[channel];

        if (writeIndexes[channel] >= bufferSize)
        {
            channelReady[channel] = true;
            bool allReady = true;
            for (auto ready : channelReady)
                if (!ready) { allReady = false; break; }

            if (allReady)
            {
                localAudioBuffer = audioBuffer;
                localAudioBufferRMS = getLocalAudioBufferRMS(0, 0, localAudioBuffer.getNumSamples());

                if (localAudioBufferRMS < rmsSilenceThreshold)
                {
                    if (silentBufferCount < bufferCountThreshold)
                    {
                        ++silentBufferCount;
                    }
                    else
                    {
                        for (auto& idx : writeIndexes) idx = 0;
                        std::fill(channelReady.begin(), channelReady.end(), false);
                        if (silentBufferCount == bufferCountThreshold)
                        {
                            ++silentBufferCount;
                            muteCallback();
                        }
                        return;
                    }
                }
                else
                {
                    if (silentBufferCount == bufferCountThreshold + 1) startCallback();
                    silentBufferCount = 0;
                }
                fullAudioBufferCallback();
                for (auto& idx : writeIndexes) idx = 0;
                std::fill(channelReady.begin(), channelReady.end(), false);
            }
        }
    }

    /* Pass in the function pointer to be called,
    and the method will return an automatically generated id.
    */
    CallbackId add(Callback cb)
    {
        std::lock_guard lock(callbacksMutex);
        CallbackId id = nextId++;
        callbacks.push_back({ id, std::move(cb) });
        return id;
    }

    /* Remove the method pointer to be called based on the id. */
    bool remove(CallbackId id)
    {
        std::lock_guard lock(callbacksMutex);
        auto it = std::find_if(callbacks.begin(), callbacks.end(),
            [id](const auto& pair) { return pair.first == id; });
        if (it != callbacks.end()) {
            callbacks.erase(it);
            return true;
        }
        return false;
    }

    /* Remove all registered callbacks */
    void removeAll()
    {
        std::lock_guard lock(callbacksMutex);
        callbacks.clear();
        this->nextId = 1;
    }

    /* Pass in the function pointer to be called,
    and the method will return an automatically generated id.
    Called after the specified number of buffered samples
    are all below the specified RMS threshold.
    */
    CallbackId addM(Callback cb)
    {
        std::lock_guard lock(callbacksMutex);
        CallbackId id = nextIdM++;
        muteCallbacks.push_back({ id, std::move(cb) });
        return id;
    }

    /* Remove the method pointer to be called based on the id. */
    bool removeM(CallbackId id)
    {
        std::lock_guard lock(callbacksMutex);
        auto it = std::find_if(muteCallbacks.begin(), muteCallbacks.end(),
            [id](const auto& pair) { return pair.first == id; });
        if (it != muteCallbacks.end()) {
            muteCallbacks.erase(it);
            return true;
        }
        return false;
    }

    /* Remove all registered callbacks */
    void removeAllM()
    {
        std::lock_guard lock(callbacksMutex);
        muteCallbacks.clear();
        this->nextIdM = 1;
    }

    /* Pass in the function pointer to be called,
    and the method will return an automatically generated id.
    Callback after the sample in the buffer reaches the threshold.
    */
    CallbackId addS(Callback cb)
    {
        std::lock_guard lock(callbacksMutex);
        CallbackId id = nextIdS++;
        startCallbacks.push_back({ id, std::move(cb) });
        return id;
    }

    /* Remove the method pointer to be called based on the id. */
    bool removeS(CallbackId id)
    {
        std::lock_guard lock(callbacksMutex);
        auto it = std::find_if(startCallbacks.begin(), startCallbacks.end(),
            [id](const auto& pair) { return pair.first == id; });
        if (it != startCallbacks.end()) {
            startCallbacks.erase(it);
            return true;
        }
        return false;
    }

    /* Remove all registered callbacks */
    void removeAllS()
    {
        std::lock_guard lock(callbacksMutex);
        startCallbacks.clear();
        this->nextId = 1;
    }

    /* Return audio buffer,But it still being written.
    If you want to use a static buffer,use getLocalAudioBuffer().
    */
    const juce::AudioBuffer<T> getAudioBuffer() const
    {
        return audioBuffer;
	}

    /* Return audio buffer read pointer,But it still being written.
    If you want to use a static buffer,use getLocalAudioBuffer().
    */
    const juce::AudioBuffer<T>* getAudioBufferReadPointer() const
    {
        return &audioBuffer;
    }

    /* Return Local Audio Buffer. */
    const juce::AudioBuffer<T> getLocalAudioBuffer() const
    {
        return localAudioBuffer;
    }

    /* Return Local Audio Buffer Read Pointer. */
    const juce::AudioBuffer<T>* getLocalAudioBufferReadPointer() const
    {
        return &localAudioBuffer;
	}

    /* Return a Local Audio Buffer reference. */
    const juce::AudioBuffer<T>& getLocalAudioBufferReference() const
    {
        return localAudioBuffer;
    }

    /* The newly added data in the process of calculating the RMS may affect 
    the returned results. */
    T getAudioBufferRMS(int channel, int startSample, int numSamples) const
    {
		return audioBuffer.getRMSLevel(channel, startSample, numSamples);
    }

    /* Return localAudioBuffer RMS,the RMS will calculating when the function 
    called. */
    T getLocalAudioBufferRMS(int channel, int startSample, int numSamples) const
    {
        return localAudioBuffer.getRMSLevel(channel, startSample, numSamples);
    }

    /* Return localAudioBuffer RMS Read Pointer. */
    template <typename T>
    const T* getLocalAudioBufferRMSReadPointer() const
    {
        return &localAudioBufferRMS;
    }

    /* Return localAudioBuffer RMS reference. */
    template <typename T>
    const T& getLocalAudioBufferRMSReference() const
    {
        return localAudioBufferRMS;
    }

    /* Return number of channels. */
    const int getAllChannels() const
    {
        return allChannels;
	}

	/* Return buffer size. */
    const int getBufferSize() const
    {
        return bufferSize;
    }

	/* Callback when the buffer is full. */
    void fullAudioBufferCallback()
    {
        std::lock_guard lock(callbacksMutex);
        auto copy = callbacks;
        for (auto& [id, cb] : copy) {
            if(cb) cb();
        }
    }

    /* Called after the specified number of buffered samples 
    are all below the specified RMS threshold. */
    void muteCallback()
    {
        std::lock_guard lock(callbacksMutex);
        auto copy = muteCallbacks;
        for (auto& [id, cb] : copy)
        {
            if (cb) cb();
        }
    }

    /* Callback after the sample in the buffer reaches the threshold. */
    void startCallback()
    {
        std::lock_guard lock(callbacksMutex);
        auto copy = startCallbacks;
        for (auto& [id, cb] : copy)
        {
            if (cb) cb();
        }
    }

private:
	juce::AudioBuffer<T> audioBuffer;
    juce::AudioBuffer<T> localAudioBuffer;
	std::vector<int> writeIndexes;
	std::vector<bool> channelReady;
	int allChannels = 0;
    int bufferSize = 0;
    CallbackId nextId = 1;
    CallbackId nextIdR = 1;
    CallbackId nextIdM = 1;
    CallbackId nextIdS = 1;
    std::vector<std::pair<CallbackId, Callback>> callbacks;
    std::vector<std::pair<CallbackId, Callback>> realTimeCallbacks;
    std::vector<std::pair<CallbackId, Callback>> muteCallbacks;
    std::vector<std::pair<CallbackId, Callback>> startCallbacks;
    std::mutex callbacksMutex;
    T localAudioBufferRMS;
    static constexpr float rmsSilenceThreshold = 0.00001f;
    int silentBufferCount = 0;
    int bufferCountThreshold = 200;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PushSampleIntoJuceAudioBuffer);
};

template <typename T>
PushSampleIntoJuceAudioBuffer<T>::PushSampleIntoJuceAudioBuffer(int allChannels, int bufferSize)
{
	writeIndexes.resize(allChannels, 0);
    channelReady.resize(allChannels);
	audioBuffer.setSize(allChannels, bufferSize);
	this->allChannels = allChannels;
	this->bufferSize = bufferSize;
}

template <typename T>
PushSampleIntoJuceAudioBuffer<T>::~PushSampleIntoJuceAudioBuffer()
{
}