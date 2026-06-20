
#include "WaveformChartComponent.h"

WaveformChartComponent::WaveformChartComponent() : rmsDataLayer(AudioLayerManager<float>::getInstance().getRMSDataLayer()), truePeak(AudioLayerManager<float>::getInstance().getTruePeak())
{
    CreateColoursConfiguration& createColoursConfiguration = CreateColoursConfiguration::getInstance();

    waveformChartCat = createColoursConfiguration.currentColourTheme
        .getChildWithProperty("name", "WaveformChart");
    lineColorL = juce::Colour(createColoursConfiguration.colourHexToARGBInt(
        waveformChartCat.getChildWithProperty("name", "BoundaryLineL").getProperty("hex").toString(), true));
    fillColorL = juce::Colour(createColoursConfiguration.colourHexToARGBInt(
        waveformChartCat.getChildWithProperty("name", "FillL").getProperty("hex").toString(), true));
    gradientColorOfLinesL = juce::Colour(createColoursConfiguration.colourHexToARGBInt(
        waveformChartCat.getChildWithProperty("name", "GradientColorOfLinesL").getProperty("hex").toString(), true));
    lineColorR = juce::Colour(createColoursConfiguration.colourHexToARGBInt(
        waveformChartCat.getChildWithProperty("name", "BoundaryLineR").getProperty("hex").toString(), true));
    fillColorR = juce::Colour(createColoursConfiguration.colourHexToARGBInt(
        waveformChartCat.getChildWithProperty("name", "FillR").getProperty("hex").toString(), true));
    gradientColorOfLinesR = juce::Colour(createColoursConfiguration.colourHexToARGBInt(
        waveformChartCat.getChildWithProperty("name", "GradientColorOfLinesR").getProperty("hex").toString(), true));

    addAndMakeVisible(&drawBounds);
	addAndMakeVisible(&componentHeader);
	addAndMakeVisible(&chartReferenceLine);

	waveformChartCat.addListener(this);

    this->totalReadSamples = truePeak->getPeaksPerBlockHyper() * 5;
}

WaveformChartComponent::~WaveformChartComponent()
{
    waveformChartCat.removeListener(this);
}

void WaveformChartComponent::clear()
{
    DBG("waveformchart buffer clear?");
}

int WaveformChartComponent::findTriggerOffset(const std::vector<TruePeak<float>::Peak>* buffer, int& firstPointIndex, int peaksPerBlock, int limitedPeaks, int targetPeriods)
{
    if (totalReadSamples < 2 || buffer == nullptr) return 0;

    int risingEdgesFound = 0;
    int lastEdgeIndex = -1000;
    firstPointIndex = 0;
    bool isFirstPoint = true;

    double audioFs = 44100.0;
    double peakFs = audioFs * (static_cast<double>(peaksPerBlock) / 1024.0);
    double cutoffHz = 100.0;

    double ff = std::tan(juce::MathConstants<double>::pi * cutoffHz / peakFs);
    double sqrt2 = std::sqrt(2.0);
    double norm = 1.0 / (1.0 + sqrt2 * ff + ff * ff);

    float b0 = static_cast<float>(ff * ff * norm);
    float b1 = static_cast<float>(2.0 * b0);
    float b2 = static_cast<float>(b0);
    float a1 = static_cast<float>(2.0 * (ff * ff - 1.0) * norm);
    float a2 = static_cast<float>((1.0 - sqrt2 * ff + ff * ff) * norm);

    TruePeak<float>::Peak latestPeak = truePeak->getPeakFromEnd(*buffer, peaksPerBlock, 0, peaksPerBlock - 1);
    float initialMid = (latestPeak.min + latestPeak.max) * 0.5f;

    float x1 = initialMid, x2 = initialMid;
    float y1 = initialMid, y2 = initialMid;
    float smoothedPrevMid = initialMid;

    for (int i = 1; i < totalReadSamples; ++i)
    {
        int blocksFromEnd = i / peaksPerBlock;
        int peakIdx = (peaksPerBlock - 1) - (i % peaksPerBlock);

        TruePeak<float>::Peak currentPeak = truePeak->getPeakFromEnd(*buffer, peaksPerBlock, blocksFromEnd, peakIdx);
        float currentMid = (currentPeak.min + currentPeak.max) * 0.5f;

        float smoothedMid = (b0 * currentMid) + (b1 * x1) + (b2 * x2) - (a1 * y1) - (a2 * y2);

        x2 = x1;
        x1 = currentMid;
        y2 = y1;
        y1 = smoothedMid;

        if (smoothedMid <= 0.0f && smoothedPrevMid > 0.0f)
        {
            if (i - lastEdgeIndex > 100)
            {
                if (isFirstPoint)
                {
                    firstPointIndex = i;
                    isFirstPoint = false;
                }

                risingEdgesFound++;
                lastEdgeIndex = i;

                if (risingEdgesFound == targetPeriods)
                {
                    int periodLength = i - firstPointIndex;
                    if (periodLength < limitedPeaks) return limitedPeaks;
                    return periodLength;
                }
            }
        }

        smoothedPrevMid = smoothedMid;
    }

    return limitedPeaks;
}

void WaveformChartComponent::paint(juce::Graphics& g)
{
    int totalBlocks = truePeak->getTotalBlocks();
    if (totalBlocks == 0 || rmsDataLayer->getTotalBlocks() == 0) return;

    int peaksPerBlock = 400;
    int lastBlocksFromEnd = -2;
    bool isFirstPointInPath = true;
    int firstPointIndex = 0;

    const std::vector<TruePeak<float>::Peak>* activeBufferL = nullptr;
    const std::vector<TruePeak<float>::Peak>* activeBufferR = nullptr;

    activeBufferL = &truePeak->getHyperResPeaks();
    activeBufferR = &truePeak->getHyperResPeaksR();
    peaksPerBlock = truePeak->getPeaksPerBlockHyper();

    float width = static_cast<float>(drawArea.getWidth());
    float height = static_cast<float>(drawArea.getHeight());
    float startX = static_cast<float>(drawArea.getX());
    float baseY = static_cast<float>(drawArea.getY());
    float absoluteRightX = startX + width - 1.0f;

    float pixelsPerBlock = width * (SINGLE_BLOCK_DURATION / currentWindow);
    float pixelsPerPeak = pixelsPerBlock / peaksPerBlock;

    int maxPeaksToDraw = juce::roundToInt(width / pixelsPerPeak);
    int totalAvailablePeaks = totalBlocks * peaksPerBlock;
    int peaksToRender = juce::jmin(maxPeaksToDraw, totalAvailablePeaks);

    if ((currentMode == waveformChartLeft || currentMode == waveformChartRight) && trigger)
    {
        const auto* triggerBuffer = (currentMode == waveformChartRight) ? activeBufferR : activeBufferL;
        peaksToRender = findTriggerOffset(triggerBuffer, firstPointIndex, peaksPerBlock, peaksPerBlock, 3);
        pixelsPerPeak = width / static_cast<float>(peaksToRender);
    }
    else
    {
        maxPeaksToDraw = juce::roundToInt(width / pixelsPerPeak);
        peaksToRender = juce::jmin(maxPeaksToDraw, totalAvailablePeaks);
        firstPointIndex = 0;
    }

    int estimatedPoints = peaksToRender * 2;

    float midY_Full = baseY + (height / 2.0f);
    float scale_Full = height / 2.0f;

    juce::Path waveformPath;
    juce::Path waveformPathR;
    juce::Path fillPath;

    waveformPath.preallocateSpace(estimatedPoints);
    waveformPathR.preallocateSpace(estimatedPoints);
    fillPath.preallocateSpace(estimatedPoints + 2);

    auto strokeCurrentPaths = [&]()
        {
            if (lastBlocksFromEnd < 0) return;

            float rmsL = rmsDataLayer->getRmsLFromEnd(lastBlocksFromEnd);
            float rmsR = rmsDataLayer->getRmsRFromEnd(lastBlocksFromEnd);
            float rmsLR = (rmsL + rmsR) / 2.0f;

            if (exchange)
            {
                if ((currentMode == waveformChartRight || currentMode == waveformChartMerge) && !waveformPathR.isEmpty())
                {
                    g.setColour(lineColorR.interpolatedWith(gradientColorOfLinesR, juce::jlimit(0.0f, 1.0f, rmsR)));
                    g.strokePath(waveformPathR, juce::PathStrokeType(1.0f));
                }

                if ((currentMode == waveformChartLeft || currentMode == waveformChartLR || currentMode == waveformChartMerge) && !waveformPath.isEmpty())
                {
                    float targetRms = (currentMode == waveformChartLR) ? rmsLR : rmsL;
                    g.setColour(lineColorL.interpolatedWith(gradientColorOfLinesL, juce::jlimit(0.0f, 1.0f, targetRms)));
                    g.strokePath(waveformPath, juce::PathStrokeType(1.0f));
                }
            }
            else
            {
                if ((currentMode == waveformChartLeft || currentMode == waveformChartLR || currentMode == waveformChartMerge) && !waveformPath.isEmpty())
                {
                    float targetRms = (currentMode == waveformChartLR) ? rmsLR : rmsL;
                    g.setColour(lineColorL.interpolatedWith(gradientColorOfLinesL, juce::jlimit(0.0f, 1.0f, targetRms)));
                    g.strokePath(waveformPath, juce::PathStrokeType(1.0f));
                }

                if ((currentMode == waveformChartRight || currentMode == waveformChartMerge) && !waveformPathR.isEmpty())
                {
                    g.setColour(lineColorR.interpolatedWith(gradientColorOfLinesR, juce::jlimit(0.0f, 1.0f, rmsR)));
                    g.strokePath(waveformPathR, juce::PathStrokeType(1.0f));
                }
            }
        };

    int renderIndex = 0;

    int endIndex = juce::jmin(firstPointIndex + peaksToRender, totalReadSamples);

    for (int x = firstPointIndex; x < endIndex; ++x)
    {
        float i = absoluteRightX - (renderIndex * pixelsPerPeak);
        if (i < startX - 10.0f) break;
        ++renderIndex;

        int currentBlocksFromEnd = x / peaksPerBlock;
        int peakIdxInBlock = (peaksPerBlock - 1) - (x % peaksPerBlock);

        if (currentBlocksFromEnd != lastBlocksFromEnd && lastBlocksFromEnd != -2)
        {
            strokeCurrentPaths();
            waveformPath.clear();
            waveformPathR.clear();
            fillPath.clear();
            isFirstPointInPath = true;
        }

        lastBlocksFromEnd = currentBlocksFromEnd;

        auto addPeakToPath = [&](juce::Path& p, float minVal, float maxVal, float anchorY, float scale) {
            float gainedMax = juce::jlimit(-1.0f, 1.0f, maxVal);
            float gainedMin = juce::jlimit(-1.0f, 1.0f, minVal);

            float yTop = anchorY - (gainedMax * scale);
            float yBottom = anchorY - (gainedMin * scale);
            if (std::abs(yTop - yBottom) < 1.0f) { yTop -= 0.5f; yBottom += 0.5f; }

            if (isFirstPointInPath) p.startNewSubPath(i, yTop);
            else p.lineTo(i, yTop);
            p.lineTo(i, yBottom);
            };

        TruePeak<float>::Peak peakL = truePeak->getPeakFromEnd(*activeBufferL, peaksPerBlock, currentBlocksFromEnd, peakIdxInBlock);
        TruePeak<float>::Peak peakR = truePeak->getPeakFromEnd(*activeBufferR, peaksPerBlock, currentBlocksFromEnd, peakIdxInBlock);

        switch (currentMode)
        {
        case waveformChartLeft:
            addPeakToPath(waveformPath, peakL.min, peakL.max, midY_Full, scale_Full);
            break;

        case waveformChartRight:
            addPeakToPath(waveformPath, peakR.min, peakR.max, midY_Full, scale_Full);
            break;

        case waveformChartLR:
        {
            float diffMax = (peakL.max - peakR.max) / 2.0f;
            float diffMin = (peakL.min - peakR.min) / 2.0f;
            addPeakToPath(waveformPath, diffMin, diffMax, midY_Full, scale_Full);
            break;
        }

        case waveformChartMerge:
            addPeakToPath(waveformPath, peakL.min, peakL.max, midY_Full, scale_Full);
            addPeakToPath(waveformPathR, peakR.min, peakR.max, midY_Full, scale_Full);
            break;
        }

        isFirstPointInPath = false;
    }
    strokeCurrentPaths();
}

void WaveformChartComponent::mouseDown(const juce::MouseEvent& event)
{
    if (event.mods.isPopupMenu())
    {
        if (cb == nullptr) return;
        cb();
    }
}

void WaveformChartComponent::mouseEnter(const juce::MouseEvent& event)
{
    drawBounds.setVisible(true);
    componentHeader.setVisible(true);
}

void WaveformChartComponent::mouseExit(const juce::MouseEvent&)
{
    if (!componentHeader.isMouseOver() && !componentHeader.headerFixedButton.getHeaderFixed())
    {
        drawBounds.setVisible(false);
        componentHeader.setVisible(false);
    }
}

void WaveformChartComponent::valueTreePropertyChanged(juce::ValueTree& tree, const juce::Identifier& property)
{
    if (property == juce::Identifier("hex"))
    {
        lineColorL = juce::Colour(CreateColoursConfiguration::getInstance().colourHexToARGBInt(
            waveformChartCat.getChildWithProperty("name", "BoundaryLineL").getProperty("hex").toString(), true));
        fillColorL = juce::Colour(CreateColoursConfiguration::getInstance().colourHexToARGBInt(
            waveformChartCat.getChildWithProperty("name", "FillL").getProperty("hex").toString(), true));
        gradientColorOfLinesL = juce::Colour(CreateColoursConfiguration::getInstance().colourHexToARGBInt(
            waveformChartCat.getChildWithProperty("name", "GradientColorOfLinesL").getProperty("hex").toString(), true));
        lineColorR = juce::Colour(CreateColoursConfiguration::getInstance().colourHexToARGBInt(
            waveformChartCat.getChildWithProperty("name", "BoundaryLineR").getProperty("hex").toString(), true));
        fillColorR = juce::Colour(CreateColoursConfiguration::getInstance().colourHexToARGBInt(
            waveformChartCat.getChildWithProperty("name", "FillR").getProperty("hex").toString(), true));
        gradientColorOfLinesR = juce::Colour(CreateColoursConfiguration::getInstance().colourHexToARGBInt(
            waveformChartCat.getChildWithProperty("name", "GradientColorOfLinesR").getProperty("hex").toString(), true));
        repaint();
    }
}