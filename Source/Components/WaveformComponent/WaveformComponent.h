#pragma once
#include <JuceHeader.h>
#include "../Source/CircularImageBuffer.h"
#include "../../CreateConfiguration/CreateColoursConfiguration.h"
#include "../../GUI/Components/DrawBounds.h"
#include "../../GUI/Components/ComponentHeader.h"
#include "WaveformComponentReferenceLine.h"

enum WaveformMode
{
    Merge,
    Separate
};

class WaveformComponent : public juce::Component, private juce::ValueTree::Listener
{
public:
    struct Peak { float min = 0.0f; float max = 0.0f; };

    WaveformComponent();
    ~WaveformComponent();

    void setWaveformData(const juce::AudioBuffer<float>& localAudioBuffer,
        const float& localAudioBufferRMS,
        const float& localAudioBufferRMSL,
        const float& localAudioBufferRMSR);

    void setTimeInterval(float seconds);

    int getRmsIndexFromPeakIndex(int peakIndex, float currentWindowSize) const;

    void paint(juce::Graphics& g) override;
    void resized() override;
    void valueTreePropertyChanged(juce::ValueTree& tree, const juce::Identifier& property);
    void mouseDown(const juce::MouseEvent& event) override;
    void mouseEnter(const juce::MouseEvent& event) override;
    void mouseExit(const juce::MouseEvent&) override;

    DrawBounds drawBounds;
    ComponentHeader componentHeader{ juce::String("Waveform") };
    WaveformComponentReferenceLine waveformReferenceLine;

    uint16_t callbackId = 0;

    using Callback = std::function<void()>;
    Callback cb = nullptr;
private:
    juce::ValueTree waveformCat;

    juce::Colour lineColorL = juce::Colours::blueviolet;
    juce::Colour gradientColorOfLinesL = juce::Colours::white;

    juce::Colour lineColorR = juce::Colours::blueviolet;
    juce::Colour gradientColorOfLinesR = juce::Colours::white;

    WaveformMode currentMode = Merge;

    void extractPeaksToTier(const float* readPtr, int totalSamples, std::vector<Peak>& tierBuffer, int numPeaksToExtract, size_t maxCapacity);
    void enforceCapacity(std::vector<Peak>& buffer, size_t maxCapacity, int numAdded);

    static constexpr int MAX_BLOCKS_HIST = 500;

    std::vector<Peak> highResPeaks;
    std::vector<Peak> highResPeaksR;
    static constexpr int PEAKS_PER_BLOCK_HIGH = 40;

    std::vector<Peak> midResPeaks;
    std::vector<Peak> midResPeaksR;
    static constexpr int PEAKS_PER_BLOCK_MID = 10;

    std::vector<Peak> lowResPeaks;
    std::vector<Peak> lowResPeaksR;
    static constexpr int PEAKS_PER_BLOCK_LOW = 3;

    std::vector<float> rmsHistory;
    std::vector<float> rmsHistoryL;
    std::vector<float> rmsHistoryR;

    float currentWindow = 10.0f;

    int64_t totalBlocksReceived = 0;

    static constexpr float SINGLE_BLOCK_DURATION = 0.021333333f;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(WaveformComponent);
};