#pragma once
#include <JuceHeader.h>
#include "../CreatePushSampleIntoJuceAudioBufferInstance.h"

template <typename T>
class RMSDataLayer
{
public:
    RMSDataLayer()
    {
        rmsHistory.resize(MAX_BLOCKS_HIST, 0.0f);
        rmsHistoryL.resize(MAX_BLOCKS_HIST, 0.0f);
        rmsHistoryR.resize(MAX_BLOCKS_HIST, 0.0f);

        auto& instance = CreatePushSampleIntoJuceAudioBufferInstance::getInstance();
        callBackId = instance.add(
            [this, &instance]() {
                rmsHistory[writeIndex] = instance.getLeftLocalAudioBufferRMSReference();
                rmsHistoryL[writeIndex] = instance.getRightLocalAudioBufferRMSReference();
                rmsHistoryR[writeIndex] = instance.getLocalAudioBufferRMSReference();

                writeIndex = (writeIndex + 1) % MAX_BLOCKS_HIST;

                if (totalBlocks < MAX_BLOCKS_HIST) {
                    totalBlocks++;
                }
            }
        );
    }

    ~RMSDataLayer()
    {
        CreatePushSampleIntoJuceAudioBufferInstance::getInstance().remove(callBackId);
    }

    T getRmsLFromEnd(int blocksFromEnd) const
    {
        if (totalBlocks == 0) return 0.0f;

        int validBlocks = juce::jmin(blocksFromEnd, totalBlocks - 1);

        int targetIdx = (writeIndex - 1 - validBlocks + MAX_BLOCKS_HIST) % MAX_BLOCKS_HIST;
        return rmsHistoryL[static_cast<size_t>(targetIdx)];
    }

    const std::vector<T>& getRMSHistory()  const { return rmsHistory; }
    const std::vector<T>& getRMSHistoryL() const { return rmsHistoryL; }
    const std::vector<T>& getRMSHistoryR() const { return rmsHistoryR; }

    int getTotalBlocks() const { return totalBlocks; }

    uint16_t callBackId;

private:
    static constexpr int MAX_BLOCKS_HIST = 500;

    std::vector<T> rmsHistory;
    std::vector<T> rmsHistoryL;
    std::vector<T> rmsHistoryR;

    int writeIndex = 0;
    int totalBlocks = 0;
};