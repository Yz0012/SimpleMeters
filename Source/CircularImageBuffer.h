#pragma once
#include <JuceHeader.h>

class CircularImageBuffer
{
public:
    explicit CircularImageBuffer(int capacity)
        : buffer(static_cast<size_t> (capacity)), writeIndex(0), count(0)
    {
        jassert(capacity > 0);
    }

    void push(const juce::Image& image)
    {
        const juce::ScopedLock sl(lock);

        buffer[writeIndex] = image;
        writeIndex = (writeIndex + 1) % buffer.size();

        if (count < static_cast<int>(buffer.size()))
            ++count;
    }

    juce::Image get(int index) const
    {
        const juce::ScopedLock sl(lock);

        return buffer[static_cast<size_t>(index)];
    }

    juce::Image getLatest() const
    {
        const juce::ScopedLock sl(lock);
        if (count == 0)
            return {};
        return get(count - 1);
    }

    int size() const
    {
        const juce::ScopedLock sl(lock);
        return buffer.size();
    }

    void clear()
    {
        const juce::ScopedLock sl(lock);
        writeIndex = 0;
        count = 0;
    }

    int getWriteIndex() const
    {
        return writeIndex;
	}

private:
    std::vector<juce::Image> buffer;
    int writeIndex = 0;
    int count = 0;
    juce::CriticalSection lock;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(CircularImageBuffer)
};