#pragma once
#include <JuceHeader.h>

class CircularImageBuffer
{
public:
    // capacity: 最多存储的图像数量
    explicit CircularImageBuffer(int capacity)
        : buffer(static_cast<size_t> (capacity)), writeIndex(0), count(0)
    {
        jassert(capacity > 0);
    }

    // 推入新图像
    void push(const juce::Image& image)
    {
        const juce::ScopedLock sl(lock);

        buffer[writeIndex] = image;
        writeIndex = (writeIndex + 1) % buffer.size();

        if (count < static_cast<int>(buffer.size()))
            ++count;
    }

    // 获取图像
    juce::Image get(int index) const
    {
        const juce::ScopedLock sl(lock);

        return buffer[static_cast<size_t>(index)];
    }

    // 获取最新的图像
    juce::Image getLatest() const
    {
        const juce::ScopedLock sl(lock);
        if (count == 0)
            return {};
        return get(count - 1);
    }

	// 获取缓冲区大小
    int size() const
    {
        const juce::ScopedLock sl(lock);
        return buffer.size();
    }

    // 清空缓冲区
    void clear()
    {
        const juce::ScopedLock sl(lock);
        writeIndex = 0;
        count = 0;
    }

    // 获取写入的索引位置
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