#pragma once
#include <JuceHeader.h>
#include "../CreatePushSampleIntoJuceAudioBufferInstance.h"

template <typename T>
class TruePeak
{
public:
    struct Peak { float min = 0.0f; float max = 0.0f; };

    TruePeak()
    {
        hyperResPeaks.resize(MAX_BLOCKS_HIST * PEAKS_PER_BLOCK_HYPER);
        hyperResPeaksR.resize(MAX_BLOCKS_HIST * PEAKS_PER_BLOCK_HYPER);

        highResPeaks.resize(MAX_BLOCKS_HIST * PEAKS_PER_BLOCK_HIGH);
        highResPeaksR.resize(MAX_BLOCKS_HIST * PEAKS_PER_BLOCK_HIGH);

        midResPeaks.resize(MAX_BLOCKS_HIST * PEAKS_PER_BLOCK_MID);
        midResPeaksR.resize(MAX_BLOCKS_HIST * PEAKS_PER_BLOCK_MID);

        midLowResPeaks.resize(MAX_BLOCKS_HIST * PEAKS_PER_BLOCK_MID_LOW);
        midLowResPeaksR.resize(MAX_BLOCKS_HIST * PEAKS_PER_BLOCK_MID_LOW);

        lowResPeaks.resize(MAX_BLOCKS_HIST * PEAKS_PER_BLOCK_LOW);
        lowResPeaksR.resize(MAX_BLOCKS_HIST * PEAKS_PER_BLOCK_LOW);

        leftTruePeak.resize(MAX_BLOCKS_HIST * PEAKS_PER_BLOCK_LOWEST);
        rightTruePeak.resize(MAX_BLOCKS_HIST * PEAKS_PER_BLOCK_LOWEST);

        auto& instance = CreatePushSampleIntoJuceAudioBufferInstance::getInstance();
        callBackId = instance.add(
            [this, &instance]() {
                int numSamples = instance.getLocalAudioBufferReference().getNumSamples();
                if (numSamples <= 0) return;

                const float* readPtr = instance.getLocalAudioBufferReference().getReadPointer(0);
                const float* readPtrR = instance.getLocalAudioBufferReference().getReadPointer(1);

                int currentBlockIdx = blockWriteIndex;

                extractPeaksToCircle(readPtr, numSamples, hyperResPeaks, PEAKS_PER_BLOCK_HYPER, currentBlockIdx);
                extractPeaksFromUpperCircle(hyperResPeaks, PEAKS_PER_BLOCK_HYPER, highResPeaks, PEAKS_PER_BLOCK_HIGH, currentBlockIdx);
                extractPeaksFromUpperCircle(highResPeaks, PEAKS_PER_BLOCK_HIGH, midResPeaks, PEAKS_PER_BLOCK_MID, currentBlockIdx);
                extractPeaksFromUpperCircle(midResPeaks, PEAKS_PER_BLOCK_MID, midLowResPeaks, PEAKS_PER_BLOCK_MID_LOW, currentBlockIdx);
                extractPeaksFromUpperCircle(midLowResPeaks, PEAKS_PER_BLOCK_MID_LOW, lowResPeaks, PEAKS_PER_BLOCK_LOW, currentBlockIdx);
                extractPeaksFromUpperCircle(lowResPeaks, PEAKS_PER_BLOCK_LOW, leftTruePeak, PEAKS_PER_BLOCK_LOWEST, currentBlockIdx);

                extractPeaksToCircle(readPtrR, numSamples, hyperResPeaksR, PEAKS_PER_BLOCK_HYPER, currentBlockIdx);
                extractPeaksFromUpperCircle(hyperResPeaksR, PEAKS_PER_BLOCK_HYPER, highResPeaksR, PEAKS_PER_BLOCK_HIGH, currentBlockIdx);
                extractPeaksFromUpperCircle(highResPeaksR, PEAKS_PER_BLOCK_HIGH, midResPeaksR, PEAKS_PER_BLOCK_MID, currentBlockIdx);
                extractPeaksFromUpperCircle(midResPeaksR, PEAKS_PER_BLOCK_MID, midLowResPeaksR, PEAKS_PER_BLOCK_MID_LOW, currentBlockIdx);
                extractPeaksFromUpperCircle(midLowResPeaksR, PEAKS_PER_BLOCK_MID_LOW, lowResPeaksR, PEAKS_PER_BLOCK_LOW, currentBlockIdx);
                extractPeaksFromUpperCircle(lowResPeaksR, PEAKS_PER_BLOCK_LOW, rightTruePeak, PEAKS_PER_BLOCK_LOWEST, currentBlockIdx);

                blockWriteIndex = (blockWriteIndex + 1) % MAX_BLOCKS_HIST;
                if (totalBlocks < MAX_BLOCKS_HIST) {
                    totalBlocks++;
                }
            }
        );
    }

    ~TruePeak()
    {
        CreatePushSampleIntoJuceAudioBufferInstance::getInstance().remove(callBackId);
    }

    Peak getPeakFromEnd(const std::vector<Peak>& buffer, int peaksPerBlock, int blocksFromEnd, int peakIdxInBlock) const
    {
        if (totalBlocks <= 0 || buffer.empty()) return { 0.0f, 0.0f };
        int validBlocks = juce::jmin(blocksFromEnd, totalBlocks - 1);
        if (validBlocks < 0) validBlocks = 0;
        int targetBlockIdx = (blockWriteIndex - 1 - validBlocks + MAX_BLOCKS_HIST) % MAX_BLOCKS_HIST;
        size_t absoluteIdx = static_cast<size_t>(targetBlockIdx * peaksPerBlock + juce::jlimit(0, peaksPerBlock - 1, peakIdxInBlock));
        if (absoluteIdx >= buffer.size()) return { 0.0f, 0.0f };
        return buffer[absoluteIdx];
    }

    const std::vector<Peak>& getHyperResPeaks()   const { return hyperResPeaks; }
    const std::vector<Peak>& getHyperResPeaksR()  const { return hyperResPeaksR; }
    const std::vector<Peak>& getHighResPeaks()    const { return highResPeaks; }
    const std::vector<Peak>& getHighResPeaksR()   const { return highResPeaksR; }
    const std::vector<Peak>& getMidResPeaks()     const { return midResPeaks; }
    const std::vector<Peak>& getMidResPeaksR()    const { return midResPeaksR; }
    const std::vector<Peak>& getMidLowResPeaks()  const { return midLowResPeaks; }
    const std::vector<Peak>& getMidLowResPeaksR() const { return midLowResPeaksR; }
    const std::vector<Peak>& getLowResPeaks()     const { return lowResPeaks; }
    const std::vector<Peak>& getLowResPeaksR()    const { return lowResPeaksR; }
    const std::vector<Peak>& getLeftTruePeak()    const { return leftTruePeak; }
    const std::vector<Peak>& getRightTruePeak()   const { return rightTruePeak; }

    int getTotalBlocks() const { return totalBlocks; }

    static constexpr int getMaxBlocksHist() { return MAX_BLOCKS_HIST; }
    static constexpr int getPeaksPerBlockHyper() { return PEAKS_PER_BLOCK_HYPER; }
    static constexpr int getPeaksPerBlockHigh() { return PEAKS_PER_BLOCK_HIGH; }
    static constexpr int getPeaksPerBlockMid() { return PEAKS_PER_BLOCK_MID; }
    static constexpr int getPeaksPerBlockMidLow() { return PEAKS_PER_BLOCK_MID_LOW; }
    static constexpr int getPeaksPerBlockLow() { return PEAKS_PER_BLOCK_LOW; }
    static constexpr int getPeaksPerBlockLowest() { return PEAKS_PER_BLOCK_LOWEST; }

    uint16_t callBackId;

private:
    void extractPeaksToCircle(const float* readPtr, int totalSamples, std::vector<Peak>& tierBuffer, int numPeaksToExtract, int blockIdx)
    {
        size_t blockStartOffset = static_cast<size_t>(blockIdx * numPeaksToExtract);
        float samplesPerSubBlock = static_cast<float>(totalSamples) / numPeaksToExtract;

        for (int i = 0; i < numPeaksToExtract; ++i)
        {
            int s0 = static_cast<int>(i * samplesPerSubBlock);
            int s1 = static_cast<int>((i + 1) * samplesPerSubBlock);
            s1 = juce::jmin(s1, totalSamples);
            if (s0 >= s1) s1 = s0 + 1;

            float minVal = 1.0f;
            float maxVal = -1.0f;

            for (int s = s0; s < s1; ++s)
            {
                float val = readPtr[s];
                if (val < minVal) minVal = val;
                if (val > maxVal) maxVal = val;
            }

            tierBuffer[blockStartOffset + i] = { minVal, maxVal };
        }
    }

    void extractPeaksFromUpperCircle(const std::vector<Peak>& upperBuffer, int numPeaksInUpperBlock, std::vector<Peak>& targetBuffer, int numPeaksToExtract, int blockIdx)
    {
        size_t upperStartOffset = static_cast<size_t>(blockIdx * numPeaksInUpperBlock);
        size_t targetStartOffset = static_cast<size_t>(blockIdx * numPeaksToExtract);
        float peaksPerSubBlock = static_cast<float>(numPeaksInUpperBlock) / numPeaksToExtract;

        for (int i = 0; i < numPeaksToExtract; ++i)
        {
            int s0 = static_cast<int>(i * peaksPerSubBlock);
            int s1 = static_cast<int>((i + 1) * peaksPerSubBlock);
            s1 = juce::jmin(s1, numPeaksInUpperBlock);
            if (s0 >= s1) s1 = s0 + 1;

            float minVal = 1.0f;
            float maxVal = -1.0f;

            for (int s = s0; s < s1; ++s)
            {
                size_t idx = upperStartOffset + static_cast<size_t>(s);
                const auto& upperPeak = upperBuffer[idx];
                if (upperPeak.min < minVal) minVal = upperPeak.min;
                if (upperPeak.max > maxVal) maxVal = upperPeak.max;
            }

            targetBuffer[targetStartOffset + i] = { minVal, maxVal };
        }
    }

    static constexpr int MAX_BLOCKS_HIST = 500;

    std::vector<Peak> hyperResPeaks;
    std::vector<Peak> hyperResPeaksR;
    static constexpr int PEAKS_PER_BLOCK_HYPER = 400;

    std::vector<Peak> highResPeaks;
    std::vector<Peak> highResPeaksR;
    static constexpr int PEAKS_PER_BLOCK_HIGH = 40;

    std::vector<Peak> midResPeaks;
    std::vector<Peak> midResPeaksR;
    static constexpr int PEAKS_PER_BLOCK_MID = 10;

    std::vector<Peak> midLowResPeaks;
    std::vector<Peak> midLowResPeaksR;
    static constexpr int PEAKS_PER_BLOCK_MID_LOW = 5;

    std::vector<Peak> lowResPeaks;
    std::vector<Peak> lowResPeaksR;
    static constexpr int PEAKS_PER_BLOCK_LOW = 3;

    std::vector<Peak> leftTruePeak;
    std::vector<Peak> rightTruePeak;
    static constexpr int PEAKS_PER_BLOCK_LOWEST = 1;

    int blockWriteIndex = 0;
    int totalBlocks = 0;
};