#include "WaveformComponent.h"

WaveformComponent::WaveformComponent()
{
    highResPeaks.reserve(MAX_BLOCKS_HIST * PEAKS_PER_BLOCK_HIGH);
    highResPeaksR.reserve(MAX_BLOCKS_HIST * PEAKS_PER_BLOCK_HIGH);
    midResPeaks.reserve(MAX_BLOCKS_HIST * PEAKS_PER_BLOCK_MID);
    midResPeaksR.reserve(MAX_BLOCKS_HIST * PEAKS_PER_BLOCK_HIGH);
    lowResPeaks.reserve(MAX_BLOCKS_HIST * PEAKS_PER_BLOCK_LOW);
    lowResPeaksR.reserve(MAX_BLOCKS_HIST * PEAKS_PER_BLOCK_HIGH);
    rmsHistory.reserve(MAX_BLOCKS_HIST);
    rmsHistoryL.reserve(MAX_BLOCKS_HIST);
    rmsHistoryR.reserve(MAX_BLOCKS_HIST);

    CreateColoursConfiguration& createColoursConfiguration = CreateColoursConfiguration::getInstance();

    waveformCat = createColoursConfiguration.currentColourTheme
        .getChildWithProperty("name", "Waveform");
    lineColorL = juce::Colour(createColoursConfiguration.colourHexToARGBInt(
        waveformCat.getChildWithProperty("name", "BoundaryLineL").getProperty("hex").toString(), true));
    gradientColorOfLinesL = juce::Colour(createColoursConfiguration.colourHexToARGBInt(
        waveformCat.getChildWithProperty("name", "GradientColorOfLinesL").getProperty("hex").toString(), true));
    lineColorR = juce::Colour(createColoursConfiguration.colourHexToARGBInt(
        waveformCat.getChildWithProperty("name", "BoundaryLineR").getProperty("hex").toString(), true));
    gradientColorOfLinesR = juce::Colour(createColoursConfiguration.colourHexToARGBInt(
        waveformCat.getChildWithProperty("name", "GradientColorOfLinesR").getProperty("hex").toString(), true));

    addAndMakeVisible(&drawBounds);
    addAndMakeVisible(&componentHeader);
    addAndMakeVisible(&waveformReferenceLine);

    waveformCat.addListener(this);
}

WaveformComponent::~WaveformComponent()
{
    waveformCat.removeListener(this);
}

void WaveformComponent::setWaveformData(const juce::AudioBuffer<float>& localAudioBuffer,
    const float& localAudioBufferRMS,
    const float& localAudioBufferRMSL,
    const float& localAudioBufferRMSR)
{
    int numSamples = localAudioBuffer.getNumSamples();
    if (numSamples <= 0) return;

    const float* readPtr = localAudioBuffer.getReadPointer(0);
    const float* readPtrR = localAudioBuffer.getReadPointer(1);

    extractPeaksToTier(readPtr, numSamples, highResPeaks, PEAKS_PER_BLOCK_HIGH, MAX_BLOCKS_HIST * PEAKS_PER_BLOCK_HIGH);
    extractPeaksToTier(readPtr, numSamples, midResPeaks, PEAKS_PER_BLOCK_MID, MAX_BLOCKS_HIST * PEAKS_PER_BLOCK_MID);
    extractPeaksToTier(readPtr, numSamples, lowResPeaks, PEAKS_PER_BLOCK_LOW, MAX_BLOCKS_HIST * PEAKS_PER_BLOCK_LOW);
    extractPeaksToTier(readPtrR, numSamples, highResPeaksR, PEAKS_PER_BLOCK_HIGH, MAX_BLOCKS_HIST * PEAKS_PER_BLOCK_HIGH);
    extractPeaksToTier(readPtrR, numSamples, midResPeaksR, PEAKS_PER_BLOCK_MID, MAX_BLOCKS_HIST * PEAKS_PER_BLOCK_MID);
    extractPeaksToTier(readPtrR, numSamples, lowResPeaksR, PEAKS_PER_BLOCK_LOW, MAX_BLOCKS_HIST * PEAKS_PER_BLOCK_LOW);

    rmsHistory.push_back(localAudioBufferRMSL);
    if (rmsHistory.size() > MAX_BLOCKS_HIST)
        rmsHistory.erase(rmsHistory.begin());

    rmsHistoryL.push_back(localAudioBufferRMSR);
    if (rmsHistoryL.size() > MAX_BLOCKS_HIST)
        rmsHistoryL.erase(rmsHistoryL.begin());

    rmsHistoryR.push_back(localAudioBufferRMSR);
    if (rmsHistoryR.size() > MAX_BLOCKS_HIST)
        rmsHistoryR.erase(rmsHistoryR.begin());

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
        activeBuffer = &lowResPeaks;  peaksPerBlock = PEAKS_PER_BLOCK_LOW;
    }

    if (activeBuffer->empty()) return;

    float width = static_cast<float>(getWidth());
    float height = static_cast<float>(getHeight());
    float midY = height / 2.0f;

    float pixelsPerBlock = width * (SINGLE_BLOCK_DURATION / currentWindow);
    float pixelsPerPeak = pixelsPerBlock / peaksPerBlock;

    int totalActivePeaks = static_cast<int>(activeBuffer->size());

    juce::Path currentBlockPath;
    int lastRmsIndex = -2;
    bool isFirstPointInPath = true;

    for (int i = 0; i < totalActivePeaks; ++i)
    {
        int bufferIdx = totalActivePeaks - 1 - i;
        const Peak& peak = (*activeBuffer)[bufferIdx];

        float x = (width - 1.0f) - (i * pixelsPerPeak);

        if (x < -10.0f)
        {
            if (!currentBlockPath.isEmpty() && lastRmsIndex >= 0)
            {
                float rmsVal = rmsHistoryL[static_cast<size_t>(lastRmsIndex)];
                g.setColour(lineColorL.interpolatedWith(gradientColorOfLinesL, juce::jlimit(0.0f, 1.0f, rmsVal)));
                g.strokePath(currentBlockPath, juce::PathStrokeType(1.0f));
            }
            break;
        }

        int currentRmsIndex = getRmsIndexFromPeakIndex(bufferIdx, currentWindow);

        if (currentRmsIndex != lastRmsIndex && lastRmsIndex != -2)
        {
            if (!currentBlockPath.isEmpty() && lastRmsIndex >= 0 && lastRmsIndex < static_cast<int>(rmsHistoryL.size()))
            {
                float rmsVal = rmsHistoryL[static_cast<size_t>(lastRmsIndex)];
                g.setColour(lineColorL.interpolatedWith(gradientColorOfLinesL, juce::jlimit(0.0f, 1.0f, rmsVal)));
                g.strokePath(currentBlockPath, juce::PathStrokeType(1.0f));
            }

            currentBlockPath.clear();
            isFirstPointInPath = true;
        }

        lastRmsIndex = currentRmsIndex;

        float yTop = midY - (peak.max * midY);
        float yBottom = midY - (peak.min * midY);
        if (std::abs(yTop - yBottom) < 1.0f) { yTop -= 0.5f; yBottom += 0.5f; }

        if (isFirstPointInPath)
        {
            currentBlockPath.startNewSubPath(x, yTop);
            isFirstPointInPath = false;
        }
        else
        {
            currentBlockPath.lineTo(x, yTop);
        }
        currentBlockPath.lineTo(x, yBottom);
    }

    if (!currentBlockPath.isEmpty() && lastRmsIndex >= 0 && lastRmsIndex < static_cast<int>(rmsHistoryL.size()))
    {
        float rmsVal = rmsHistoryL[static_cast<size_t>(lastRmsIndex)];
        g.setColour(lineColorL.interpolatedWith(gradientColorOfLinesL, juce::jlimit(0.0f, 1.0f, rmsVal)));
        g.strokePath(currentBlockPath, juce::PathStrokeType(1.0f));
    }
}

int WaveformComponent::getRmsIndexFromPeakIndex(int peakIndex, float currentWindowSize) const
{
    if (rmsHistoryL.empty()) return -1;

    int ratio = (currentWindowSize < 2.0f) ? PEAKS_PER_BLOCK_HIGH
        : ((currentWindowSize < 5.0f) ? PEAKS_PER_BLOCK_MID : PEAKS_PER_BLOCK_LOW);

    int activeBufferSize = 0;
    if (currentWindowSize < 2.0f)      activeBufferSize = static_cast<int>(highResPeaks.size());
    else if (currentWindowSize < 5.0f) activeBufferSize = static_cast<int>(midResPeaks.size());
    else                               activeBufferSize = static_cast<int>(lowResPeaks.size());

    if (activeBufferSize == 0) return -1;

    // 倒序推算：当前 peakIndex 距离 Buffer 尾部（最新数据）有多少个点
    int stepsFromEnd = activeBufferSize - 1 - peakIndex;

    // 折算成 Block 数量
    int rmsStepsFromEnd = stepsFromEnd / ratio;

    // 从 RMS 历史的尾部向前推
    int rmsIndex = static_cast<int>(rmsHistoryL.size() - 1) - rmsStepsFromEnd;

    return juce::jlimit(0, static_cast<int>(rmsHistoryL.size() - 1), rmsIndex);
}

void WaveformComponent::valueTreePropertyChanged(juce::ValueTree& tree, const juce::Identifier& property)
{
    if (property == juce::Identifier("hex"))
    {
        lineColorL = juce::Colour(CreateColoursConfiguration::getInstance().colourHexToARGBInt(
            waveformCat.getChildWithProperty("name", "BoundaryLineL").getProperty("hex").toString(), true));
        gradientColorOfLinesL = juce::Colour(CreateColoursConfiguration::getInstance().colourHexToARGBInt(
            waveformCat.getChildWithProperty("name", "GradientColorOfLinesL").getProperty("hex").toString(), true));
        lineColorR = juce::Colour(CreateColoursConfiguration::getInstance().colourHexToARGBInt(
            waveformCat.getChildWithProperty("name", "BoundaryLineR").getProperty("hex").toString(), true));
        gradientColorOfLinesR = juce::Colour(CreateColoursConfiguration::getInstance().colourHexToARGBInt(
            waveformCat.getChildWithProperty("name", "GradientColorOfLinesR").getProperty("hex").toString(), true));
        repaint();
    }
}

void WaveformComponent::mouseDown(const juce::MouseEvent& event)
{
    if (event.mods.isPopupMenu())
    {
        if (cb == nullptr) return;
        cb();
    }
}

void WaveformComponent::mouseEnter(const juce::MouseEvent& event)
{
    drawBounds.setVisible(true);
    componentHeader.setVisible(true);
}

void WaveformComponent::mouseExit(const juce::MouseEvent&)
{
    if (!componentHeader.isMouseOver() && !componentHeader.headerFixedButton.getHeaderFixed())
    {
        drawBounds.setVisible(false);
        componentHeader.setVisible(false);
    }
}