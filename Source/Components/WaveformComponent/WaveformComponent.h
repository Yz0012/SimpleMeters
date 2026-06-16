#pragma once
#include <JuceHeader.h>
#include <vector>

class WaveformComponent : public juce::Component
{
public:
    struct Peak { float min = 0.0f; float max = 0.0f; };

    WaveformComponent();
    ~WaveformComponent() override = default;

    void setWaveformData(const juce::AudioBuffer<float>& localAudioBuffer,
        const float& localAudioBufferRMS);

    void setTimeInterval(float seconds);

    int getRmsIndexFromPeakIndex(int peakIndex, float currentWindowSize) const;

    void paint(juce::Graphics& g) override;
    void resized() override;

    uint16_t callbackId = 0;
private:
    void extractPeaksToTier(const float* readPtr, int totalSamples, std::vector<Peak>& tierBuffer, int numPeaksToExtract, size_t maxCapacity);
    void enforceCapacity(std::vector<Peak>& buffer, size_t maxCapacity, int numAdded);

    static constexpr int MAX_BLOCKS_HIST = 500;

    std::vector<Peak> highResPeaks;
    static constexpr int PEAKS_PER_BLOCK_HIGH = 40;

    std::vector<Peak> midResPeaks;
    static constexpr int PEAKS_PER_BLOCK_MID = 10;

    std::vector<Peak> lowResPeaks;
    static constexpr int PEAKS_PER_BLOCK_LOW = 2;

    std::vector<float> rmsHistory;

    float currentWindow = 7.5f;

    int64_t totalBlocksReceived = 0;

    static constexpr float SINGLE_BLOCK_DURATION = 0.02f;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(WaveformComponent);
};