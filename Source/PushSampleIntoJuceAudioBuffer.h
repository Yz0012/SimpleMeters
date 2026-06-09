
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
        buffers[currentWriteIdx].setSample(channel, writeIndexes[channel], sample);
        ++writeIndexes[channel];

        if (writeIndexes[channel] >= bufferSize)
        {
            channelReady[channel] = true;
            bool allReady = true;
            for (auto ready : channelReady)
                if (!ready) { allReady = false; break; }

            /* Although computation should not be placed in the audio thread, about 1000 float samples will not block the audio thread.
            The computation process on modern CPUs only takes a few microseconds.
            Considering that it is possible to implement automatic start and stop callbacks, I think it is worth it. */

            if (channel)
            {
                rightLocalAudioBufferRMS = buffers[channel].getRMSLevel(channel, 0, bufferSize);
            }
            else
            {
				leftLocalAudioBufferRMS = buffers[channel].getRMSLevel(channel, 0, bufferSize);
            }

            if (allReady)
            {
                localAudioBufferRMS = sqrtf(leftLocalAudioBufferRMS * leftLocalAudioBufferRMS +
                    rightLocalAudioBufferRMS * rightLocalAudioBufferRMS);
                // Thread-safe
                activeReadIndex.store(currentWriteIdx, std::memory_order_release);
                currentWriteIdx = 1 - currentWriteIdx;

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
                            juce::MessageManager::callAsync([this] { muteCallback(); });
                        }
                        return;
                    }
                }
                else
                {
                    if (silentBufferCount == bufferCountThreshold + 1) juce::MessageManager::callAsync([this] { startCallback(); });;
                    silentBufferCount = 0;
                }
                juce::MessageManager::callAsync([this] { fullAudioBufferCallback(); });
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
        std::lock_guard lock(fullCallbacksMutex);
        CallbackId id = nextId++;
        callbacks.push_back({ id, std::move(cb) });
        return id;
    }

    /* Remove the method pointer to be called based on the id. */
    bool remove(CallbackId id)
    {
        std::lock_guard lock(fullCallbacksMutex);
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
        std::lock_guard lock(fullCallbacksMutex);
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
        std::lock_guard lock(muteCallbacksMutex);
        CallbackId id = nextIdM++;
        muteCallbacks.push_back({ id, std::move(cb) });
        return id;
    }

    /* Remove the method pointer to be called based on the id. */
    bool removeM(CallbackId id)
    {
        std::lock_guard lock(muteCallbacksMutex);
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
        std::lock_guard lock(muteCallbacksMutex);
        muteCallbacks.clear();
        this->nextIdM = 1;
    }

    /* Pass in the function pointer to be called,
    and the method will return an automatically generated id.
    Callback after the sample in the buffer reaches the threshold.
    */
    CallbackId addS(Callback cb)
    {
        std::lock_guard lock(startCallbacksMutex);
        CallbackId id = nextIdS++;
        startCallbacks.push_back({ id, std::move(cb) });
        return id;
    }

    /* Remove the method pointer to be called based on the id. */
    bool removeS(CallbackId id)
    {
        std::lock_guard lock(startCallbacksMutex);
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
        std::lock_guard lock(startCallbacksMutex);
        startCallbacks.clear();
        this->nextIdS = 1;
    }

    /* Return audio buffer,But it may still being written.
    If you want to use a static buffer,use getLocalAudioBuffer().
    You'd better not use this thing. */
    const juce::AudioBuffer<T> getAudioBuffer(int idx) const
    {
        return buffers[idx];
	}

    /* Return audio buffer read pointer,But it still being written.
    If you want to use a static buffer,use getLocalAudioBuffer().
    You'd better not use this thing. */
    const juce::AudioBuffer<T>* getAudioBufferReadPointer(int idx) const
    {
        return &buffers[idx];
    }

    /* Return Local Audio Buffer. */
    const juce::AudioBuffer<T> getLocalAudioBuffer() const
    {
        int idx = activeReadIndex.load(std::memory_order_acquire);
        return buffers[idx];
    }

    /* Return Local Audio Buffer Read Pointer. */
    const juce::AudioBuffer<T>* getLocalAudioBufferReadPointer() const
    {
        int idx = activeReadIndex.load(std::memory_order_acquire);
        return &buffers[idx];
	}

    /* Return a Local Audio Buffer reference. */
    const juce::AudioBuffer<T>& getLocalAudioBufferReference() const
    {
        int idx = activeReadIndex.load(std::memory_order_acquire);
        return buffers[idx];
    }

    /* The newly added data in the process of calculating the RMS may affect 
    the returned results.
    You'd better not use this thing. */
    T getAudioBufferRMS(int channel, int startSample, int numSamples, int idx) const
    {
		return buffers[idx].getRMSLevel(channel, startSample, numSamples);
    }

    /* Return localAudioBuffer RMS, the RMS will be calculated when the function is called. */
    T getLocalAudioBufferRMS(int channel, int startSample, int numSamples) const
    {
        int idx = activeReadIndex.load(std::memory_order_acquire);
        return buffers[idx].getRMSLevel(channel, startSample, numSamples);
    }

    /* Return localAudioBuffer RMS (stored atomic value). */
    T getLocalAudioBufferRMS() const
    {
        return localAudioBufferRMS.load(std::memory_order_acquire);
    }

    /* Return localAudioBuffer RMS (stored atomic value) – note: returns by value, NOT reference. */
    T getLocalAudioBufferRMSReference() const
    {
        return localAudioBufferRMS.load(std::memory_order_acquire);
    }

    /* Return left channel RMS of localAudioBuffer, the RMS will be calculated when the function is called. */
    T getLeftLocalAudioBufferRMS(int startSample, int numSamples) const
    {
        int idx = activeReadIndex.load(std::memory_order_acquire);
        return buffers[idx].getRMSLevel(0, startSample, numSamples);
    }

    /* Return left channel RMS of localAudioBuffer (stored atomic value). */
    T getLeftLocalAudioBufferRMS() const
    {
        return leftLocalAudioBufferRMS.load(std::memory_order_acquire);
    }

    /* Return left channel RMS of localAudioBuffer (stored atomic value) – returns by value. */
    T getLeftLocalAudioBufferRMSReference() const
    {
        return leftLocalAudioBufferRMS.load(std::memory_order_acquire);
    }

    /* Return right channel RMS of localAudioBuffer, the RMS will be calculated when the function is called. */
    T getRightLocalAudioBufferRMS(int startSample, int numSamples) const
    {
        int idx = activeReadIndex.load(std::memory_order_acquire);
        return buffers[idx].getRMSLevel(1, startSample, numSamples);
    }

    /* Return right channel RMS of localAudioBuffer (stored atomic value). */
    T getRightLocalAudioBufferRMS() const
    {
        return rightLocalAudioBufferRMS.load(std::memory_order_acquire);
    }

    /* Return right channel RMS of localAudioBuffer (stored atomic value) – returns by value. */
    T getRightLocalAudioBufferRMSReference() const
    {
        return rightLocalAudioBufferRMS.load(std::memory_order_acquire);
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

private:
    /* Callback when the buffer is full. */
    void fullAudioBufferCallback()
    {
        std::lock_guard lock(fullCallbacksMutex);
        auto copy = callbacks;
        for (auto& [id, cb] : copy) {
            if (cb) cb();
        }
    }

    /* Called after the specified number of buffered samples
    are all below the specified RMS threshold. */
    void muteCallback()
    {
        std::lock_guard lock(muteCallbacksMutex);
        auto copy = muteCallbacks;
        for (auto& [id, cb] : copy)
        {
            if (cb) cb();
        }
    }

    /* Callback after the sample in the buffer reaches the threshold. */
    void startCallback()
    {
        std::lock_guard lock(startCallbacksMutex);
        auto copy = startCallbacks;
        for (auto& [id, cb] : copy)
        {
            if (cb) cb();
        }
    }

	std::vector<int> writeIndexes;
	std::vector<bool> channelReady;
	int allChannels = 0;
    int bufferSize = 0;
    CallbackId nextId = 1;
    CallbackId nextIdM = 1;
    CallbackId nextIdS = 1;
    std::vector<std::pair<CallbackId, Callback>> callbacks;
    std::vector<std::pair<CallbackId, Callback>> muteCallbacks;
    std::vector<std::pair<CallbackId, Callback>> startCallbacks;
    std::mutex fullCallbacksMutex;
    std::mutex muteCallbacksMutex;
    std::mutex startCallbacksMutex;
    std::atomic<T> localAudioBufferRMS;
	std::atomic<T> leftLocalAudioBufferRMS;
	std::atomic<T> rightLocalAudioBufferRMS;
    static constexpr float rmsSilenceThreshold = 0.00001f;
    int silentBufferCount = 0;
    int bufferCountThreshold = 200;
    std::array<juce::AudioBuffer<T>, 2> buffers;
    std::atomic<int> activeReadIndex{ 0 };
    int currentWriteIdx = 1;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PushSampleIntoJuceAudioBuffer);
};

template <typename T>
PushSampleIntoJuceAudioBuffer<T>::PushSampleIntoJuceAudioBuffer(int allChannels, int bufferSize)
    : allChannels(allChannels), bufferSize(bufferSize),
    writeIndexes(allChannels, 0), channelReady(allChannels, false)
{
    for (auto& buf : buffers) buf.setSize(allChannels, bufferSize);
    buffers[0].clear();
    activeReadIndex.store(0, std::memory_order_release);
}

template <typename T>
PushSampleIntoJuceAudioBuffer<T>::~PushSampleIntoJuceAudioBuffer()
{
}