#include "WaveformComponent.h"

WaveformComponent::WaveformComponent()
{
    highResPeaks.reserve(MAX_BLOCKS_HIST * PEAKS_PER_BLOCK_HIGH);
    midResPeaks.reserve(MAX_BLOCKS_HIST * PEAKS_PER_BLOCK_MID);
    lowResPeaks.reserve(MAX_BLOCKS_HIST * PEAKS_PER_BLOCK_LOW);
    rmsHistory.reserve(MAX_BLOCKS_HIST);
}

void WaveformComponent::setWaveformData(const juce::AudioBuffer<float>& localAudioBuffer,
    const float& localAudioBufferRMS)
{
    int numSamples = localAudioBuffer.getNumSamples();
    if (numSamples <= 0) return;

    const float* readPtr = localAudioBuffer.getReadPointer(0);

    extractPeaksToTier(readPtr, numSamples, highResPeaks, PEAKS_PER_BLOCK_HIGH, MAX_BLOCKS_HIST * PEAKS_PER_BLOCK_HIGH);
    extractPeaksToTier(readPtr, numSamples, midResPeaks, PEAKS_PER_BLOCK_MID, MAX_BLOCKS_HIST * PEAKS_PER_BLOCK_MID);
    extractPeaksToTier(readPtr, numSamples, lowResPeaks, PEAKS_PER_BLOCK_LOW, MAX_BLOCKS_HIST * PEAKS_PER_BLOCK_LOW);

    rmsHistory.push_back(localAudioBufferRMS);
    if (rmsHistory.size() > MAX_BLOCKS_HIST)
        rmsHistory.erase(rmsHistory.begin());

    repaint();
}

void WaveformComponent::setTimeInterval(float seconds)
{
    currentWindow = juce::jlimit(0.5f, 10.0f, seconds);
    repaint();
}

void WaveformComponent::resized()
{
    repaint();
}

void WaveformComponent::extractPeaksToTier(const float* readPtr, int totalSamples,
    std::vector<Peak>& tierBuffer,
    int numPeaksToExtract, size_t maxCapacity)
{
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

        tierBuffer.push_back({ minVal, maxVal });
    }

    if (tierBuffer.size() > maxCapacity)
    {
        size_t eraseCount = tierBuffer.size() - maxCapacity;
        tierBuffer.erase(tierBuffer.begin(), tierBuffer.begin() + eraseCount);
    }
}

void WaveformComponent::paint(juce::Graphics& g)
{

    const std::vector<Peak>* activeBuffer = nullptr;
    int peaksPerBlock = 1;

    if (currentWindow < 2.0f) {
        activeBuffer = &highResPeaks; peaksPerBlock = PEAKS_PER_BLOCK_HIGH;
    }
    else if (currentWindow < 5.0f) {
        activeBuffer = &midResPeaks;  peaksPerBlock = PEAKS_PER_BLOCK_MID;
    }
    else {
        activeBuffer = &lowResPeaks;  \
            peaksPerBlock = PEAKS_PER_BLOCK_LOW;
    }

    if (activeBuffer->empty()) return;

    float width = static_cast<float>(getWidth());
    float height = static_cast<float>(getHeight());
    float midY = height / 2.0f;

    float pixelsPerBlock = width * (SINGLE_BLOCK_DURATION / currentWindow);

    float pixelsPerPeak = pixelsPerBlock / peaksPerBlock;

    juce::Path waveformPath;
    int totalActivePeaks = static_cast<int>(activeBuffer->size());

    for (int i = 0; i < totalActivePeaks; ++i)
    {
        int bufferIdx = totalActivePeaks - 1 - i;
        const Peak& peak = (*activeBuffer)[bufferIdx];

        float x = (width - 1.0f) - (i * pixelsPerPeak);

        if (x < -5.0f) break;

        float yTop = midY - (peak.max * midY);
        float yBottom = midY - (peak.min * midY);

        if (std::abs(yTop - yBottom) < 1.0f)
        {
            yTop -= 0.5f;
            yBottom += 0.5f;
        }

        if (i == 0)
        {
            waveformPath.startNewSubPath(x, yTop);
        }
        else
        {
            waveformPath.lineTo(x, yTop);
        }
        waveformPath.lineTo(x, yBottom);
    }

    g.setColour(juce::Colours::lightgreen);
    g.strokePath(waveformPath, juce::PathStrokeType(1.0f));
}

int WaveformComponent::getRmsIndexFromPeakIndex(int peakIndex, float currentWindowSize) const
{
    if (rmsHistory.empty()) return -1;
    int ratio = (currentWindowSize < 2.0f) ? PEAKS_PER_BLOCK_HIGH : ((currentWindowSize < 5.0f) ? PEAKS_PER_BLOCK_MID : PEAKS_PER_BLOCK_LOW);

    int activeBufferSize = 0;
    if (currentWindowSize < 2.0f)      activeBufferSize = highResPeaks.size();
    else if (currentWindowSize < 5.0f) activeBufferSize = midResPeaks.size();
    else                               activeBufferSize = lowResPeaks.size();

    if (activeBufferSize == 0) return -1;

    int stepsFromEnd = activeBufferSize - 1 - peakIndex;
    int rmsStepsFromEnd = stepsFromEnd / ratio;
    int rmsIndex = static_cast<int>(rmsHistory.size() - 1) - rmsStepsFromEnd;

    return juce::jlimit(0, static_cast<int>(rmsHistory.size() - 1), rmsIndex);
}