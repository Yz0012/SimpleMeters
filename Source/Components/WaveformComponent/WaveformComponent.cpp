#include "WaveformComponent.h"

WaveformComponent::WaveformComponent() : rmsDataLayer(AudioLayerManager<float>::getInstance().getRMSDataLayer()), truePeak(AudioLayerManager<float>::getInstance().getTruePeak())
{

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

    componentHeader.addAndMakeVisible(&componentHeader.knob);
    componentHeader.addAndMakeVisible(&componentHeader.knobTwo);

    waveformCat.addListener(this);
    componentHeader.knob.setDoubleClickReturnValue(true, 0.5f);
    componentHeader.knob.setRange(0.5f, 10.0f);
    componentHeader.knob.setColour(juce::Slider::textBoxOutlineColourId, juce::Colour(0x00000000));
    componentHeader.knob.setColour(juce::Slider::textBoxBackgroundColourId, juce::Colour(0x00000000));
    componentHeader.knob.setColour(juce::Slider::textBoxTextColourId, juce::Colour(0xFFB7ED88));
    sliderValueChanged(&componentHeader.knob);
    componentHeader.knob.addListener(this);

    componentHeader.knobTwo.setDoubleClickReturnValue(true, 0.0f);
    componentHeader.knobTwo.setRange(-10.0f, 10.0f);
    componentHeader.knobTwo.setColour(juce::Slider::textBoxOutlineColourId, juce::Colour(0x00000000));
    componentHeader.knobTwo.setColour(juce::Slider::textBoxBackgroundColourId, juce::Colour(0x00000000));
    componentHeader.knobTwo.setColour(juce::Slider::textBoxTextColourId, juce::Colour(0xFFB7ED88));
    componentHeader.knobTwo.addListener(this);
}

WaveformComponent::~WaveformComponent()
{
    waveformCat.removeListener(this);
    componentHeader.knob.removeListener(this);
    componentHeader.knobTwo.removeListener(this);
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

void WaveformComponent::paint(juce::Graphics& g)
{
    const std::vector<TruePeak<float>::Peak>* activeBufferL = nullptr;
    const std::vector<TruePeak<float>::Peak>* activeBufferR = nullptr;
    int peaksPerBlock = 1;

    if (currentWindow < 2.0f) {
        activeBufferL = &truePeak->getHighResPeaks();
        activeBufferR = &truePeak->getHighResPeaksR();
        peaksPerBlock = truePeak->getPeaksPerBlockHigh();
    }
    else if (currentWindow < 5.0f) {
        activeBufferL = &truePeak->getMidResPeaks();
        activeBufferR = &truePeak->getMidResPeaksR();
        peaksPerBlock = truePeak->getPeaksPerBlockMid();
    }
    else if (currentWindow < 8.0f) {
        activeBufferL = &truePeak->getMidLowResPeaks();
        activeBufferR = &truePeak->getMidLowResPeaksR();
        peaksPerBlock = truePeak->getPeaksPerBlockMidLow();
    }
    else {
        activeBufferL = &truePeak->getLowResPeaks();
        activeBufferR = &truePeak->getLowResPeaksR();
        peaksPerBlock = truePeak->getPeaksPerBlockLow();
    }

    int totalBlocks = truePeak->getTotalBlocks();
    if (totalBlocks == 0 || rmsDataLayer->getTotalBlocks() == 0) return;

    float width = static_cast<float>(drawArea.getWidth());
    float height = static_cast<float>(drawArea.getHeight());
    float startX = static_cast<float>(drawArea.getX());
    float baseY = static_cast<float>(drawArea.getY());

    float pixelsPerBlock = width * (SINGLE_BLOCK_DURATION / currentWindow);
    float pixelsPerPeak = pixelsPerBlock / peaksPerBlock;

    int maxPeaksToDraw = juce::roundToInt(width / pixelsPerPeak);
    int totalAvailablePeaks = totalBlocks * peaksPerBlock;
    int peaksToRender = juce::jmin(maxPeaksToDraw, totalAvailablePeaks);

    juce::Path pathL;
    juce::Path pathR;

    int estimatedPoints = peaksToRender * 2;

    pathL.preallocateSpace(estimatedPoints);
    pathR.preallocateSpace(estimatedPoints);

    int lastBlocksFromEnd = -2;
    bool isFirstPointInPath = true;

    float absoluteRightX = startX + width - 1.0f;

    auto strokeCurrentPaths = [&]()
        {
            if (lastBlocksFromEnd < 0) return;

            float rmsL = rmsDataLayer->getRmsLFromEnd(lastBlocksFromEnd);
            float rmsR = rmsDataLayer->getRmsRFromEnd(lastBlocksFromEnd);
            float rmsLR = (rmsL + rmsR) / 2.0f;

            if (exchange)
            {
                if ((currentMode == waveformRight || currentMode == waveformMerge || currentMode == waveformSeparate) && !pathR.isEmpty())
                {
                    g.setColour(lineColorR.interpolatedWith(gradientColorOfLinesR, juce::jlimit(0.0f, 1.0f, rmsR)));
                    g.strokePath(pathR, juce::PathStrokeType(1.0f));
                }

                if ((currentMode == waveformLeft || currentMode == waveformLR || currentMode == waveformMerge || currentMode == waveformSeparate) && !pathL.isEmpty())
                {
                    float targetRms = (currentMode == waveformLR) ? rmsLR : rmsL;
                    g.setColour(lineColorL.interpolatedWith(gradientColorOfLinesL, juce::jlimit(0.0f, 1.0f, targetRms)));
                    g.strokePath(pathL, juce::PathStrokeType(1.0f));
                }
            }
            else
            {
                if ((currentMode == waveformLeft || currentMode == waveformLR || currentMode == waveformMerge || currentMode == waveformSeparate) && !pathL.isEmpty())
                {
                    float targetRms = (currentMode == waveformLR) ? rmsLR : rmsL;
                    g.setColour(lineColorL.interpolatedWith(gradientColorOfLinesL, juce::jlimit(0.0f, 1.0f, targetRms)));
                    g.strokePath(pathL, juce::PathStrokeType(1.0f));
                }

                if ((currentMode == waveformRight || currentMode == waveformMerge || currentMode == waveformSeparate) && !pathR.isEmpty())
                {
                    g.setColour(lineColorR.interpolatedWith(gradientColorOfLinesR, juce::jlimit(0.0f, 1.0f, rmsR)));
                    g.strokePath(pathR, juce::PathStrokeType(1.0f));
                }
            }
        };

    float midY_Full = baseY + (height / 2.0f);
    float scale_Full = height / 2.0f;

    float midY_Top = baseY + (height / 4.0f);
    float midY_Bottom = baseY + (height * 0.75f);
    float scale_Half = height / 4.0f;

    for (int i = 0; i < peaksToRender; ++i)
    {
        float x = absoluteRightX - (i * pixelsPerPeak);
        if (x < startX - 10.0f) break;

        int currentBlocksFromEnd = i / peaksPerBlock;
        int peakIdxInBlock = (peaksPerBlock - 1) - (i % peaksPerBlock);

        if (currentBlocksFromEnd != lastBlocksFromEnd && lastBlocksFromEnd != -2)
        {
            strokeCurrentPaths();
            pathL.clear();
            pathR.clear();
            isFirstPointInPath = true;
        }

        lastBlocksFromEnd = currentBlocksFromEnd;

        auto addPeakToPath = [&](juce::Path& p, float minVal, float maxVal, float anchorY, float scale) {
            float gainedMax = juce::jlimit(-1.0f, 1.0f, maxVal * gain);
            float gainedMin = juce::jlimit(-1.0f, 1.0f, minVal * gain);

            float yTop = anchorY - (gainedMax * scale);
            float yBottom = anchorY - (gainedMin * scale);
            if (std::abs(yTop - yBottom) < 1.0f) { yTop -= 0.5f; yBottom += 0.5f; }

            if (isFirstPointInPath) p.startNewSubPath(x, yTop);
            else p.lineTo(x, yTop);
            p.lineTo(x, yBottom);
            };

        TruePeak<float>::Peak peakL = truePeak->getPeakFromEnd(*activeBufferL, peaksPerBlock, currentBlocksFromEnd, peakIdxInBlock);
        TruePeak<float>::Peak peakR = truePeak->getPeakFromEnd(*activeBufferR, peaksPerBlock, currentBlocksFromEnd, peakIdxInBlock);

        switch (currentMode)
        {
        case waveformLeft:
            addPeakToPath(pathL, peakL.min, peakL.max, midY_Full, scale_Full);
            break;

        case waveformRight:
            addPeakToPath(pathR, peakR.min, peakR.max, midY_Full, scale_Full);
            break;

        case waveformLR:
        {
            float diffMax = (peakL.max - peakR.max) / 2.0f;
            float diffMin = (peakL.min - peakR.min) / 2.0f;
            addPeakToPath(pathL, diffMin, diffMax, midY_Full, scale_Full);
            break;
        }

        case waveformMerge:
            addPeakToPath(pathL, peakL.min, peakL.max, midY_Full, scale_Full);
            addPeakToPath(pathR, peakR.min, peakR.max, midY_Full, scale_Full);
            break;

        case waveformSeparate:
            addPeakToPath(pathL, peakL.min, peakL.max, midY_Top, scale_Half);
            addPeakToPath(pathR, peakR.min, peakR.max, midY_Bottom, scale_Half);
            break;
        }

        isFirstPointInPath = false;
    }

    strokeCurrentPaths();
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

void WaveformComponent::sliderValueChanged(juce::Slider* slider)
{
    if (slider == &componentHeader.knob)
    {
        currentWindow = slider->getValue();
        waveformReferenceLine.setTimeWindow(currentWindow);
        repaint();
    }
    else if (slider == &componentHeader.knobTwo)
    {
        gain = std::pow(10.0f, slider->getValue() / 20.0f);
        waveformReferenceLine.setGainDb(slider->getValue());
        repaint();
    }
}

void WaveformComponent::setWaveformMode(WaveformMode mode)
{
    if (currentMode != mode)
    {
        currentMode = mode;
    }
}

void WaveformComponent::exchangeLRposition()
{
    this->exchange = !exchange;
}