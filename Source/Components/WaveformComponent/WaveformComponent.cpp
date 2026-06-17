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

    waveformCat.addListener(this);
    componentHeader.knob.setDoubleClickReturnValue(true, 0.5f);
    componentHeader.knob.setRange(0.5f, 10.0f);
    sliderValueChanged(&componentHeader.knob);
    componentHeader.knob.addListener(this);
}

WaveformComponent::~WaveformComponent()
{
    waveformCat.removeListener(this);
    componentHeader.knob.removeListener(this);
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
    const std::vector<TruePeak<float>::Peak>* activeBuffer = nullptr;
    int peaksPerBlock = 1;

    if (currentWindow < 2.0f) {
        activeBuffer = &truePeak->getHighResPeaks(); peaksPerBlock = truePeak->getPeaksPerBlockHigh();
    }
    else if (currentWindow < 5.0f) {
        activeBuffer = &truePeak->getMidResPeaks();  peaksPerBlock = truePeak->getPeaksPerBlockMid();
    }
    else {
        activeBuffer = &truePeak->getLowResPeaks();  peaksPerBlock = truePeak->getPeaksPerBlockLow();
    }

    int totalBlocks = truePeak->getTotalBlocks();
    if (totalBlocks == 0 || rmsDataLayer->getTotalBlocks() == 0) return;

    float width = static_cast<float>(drawArea.getWidth());
    float height = static_cast<float>(drawArea.getHeight());
    float midY = drawArea.getY() + (height / 2.0f);
    float startX = static_cast<float>(drawArea.getX());

    float pixelsPerBlock = width * (SINGLE_BLOCK_DURATION / currentWindow);
    float pixelsPerPeak = pixelsPerBlock / peaksPerBlock;

    int maxPeaksToDraw = juce::roundToInt(width / pixelsPerPeak);
    int totalAvailablePeaks = totalBlocks * peaksPerBlock;
    int peaksToRender = juce::jmin(maxPeaksToDraw, totalAvailablePeaks);

    juce::Path currentBlockPath;
    int lastBlocksFromEnd = -2;
    bool isFirstPointInPath = true;

    float absoluteRightX = startX + width - 1.0f;

    for (int i = 0; i < peaksToRender; ++i)
    {

        float x = absoluteRightX - (i * pixelsPerPeak);
        if (x < startX - 10.0f) break;

        int currentBlocksFromEnd = i / peaksPerBlock;
        int peakIdxInBlock = (peaksPerBlock - 1) - (i % peaksPerBlock);

        TruePeak<float>::Peak peak = truePeak->getPeakFromEnd(*activeBuffer, peaksPerBlock, currentBlocksFromEnd, peakIdxInBlock);

        if (currentBlocksFromEnd != lastBlocksFromEnd && lastBlocksFromEnd != -2)
        {
            if (!currentBlockPath.isEmpty() && lastBlocksFromEnd >= 0)
            {
                float rmsVal = rmsDataLayer->getRmsLFromEnd(lastBlocksFromEnd);
                g.setColour(lineColorL.interpolatedWith(gradientColorOfLinesL, juce::jlimit(0.0f, 1.0f, rmsVal)));
                g.strokePath(currentBlockPath, juce::PathStrokeType(1.0f));
            }

            currentBlockPath.clear();
            isFirstPointInPath = true;
        }

        lastBlocksFromEnd = currentBlocksFromEnd;

        float yTop = midY - (peak.max * (height / 2.0f));
        float yBottom = midY - (peak.min * (height / 2.0f));
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

    if (!currentBlockPath.isEmpty() && lastBlocksFromEnd >= 0)
    {
        float rmsVal = rmsDataLayer->getRmsLFromEnd(lastBlocksFromEnd);
        g.setColour(lineColorL.interpolatedWith(gradientColorOfLinesL, juce::jlimit(0.0f, 1.0f, rmsVal)));
        g.strokePath(currentBlockPath, juce::PathStrokeType(1.0f));
    }
}

int WaveformComponent::getRmsIndexFromPeakIndex(int peakIndex, float peaksPerBlock) const
{
    return peakIndex / peaksPerBlock;
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
    this->setTimeInterval(slider->getValue());
}