
#include "WaveformChartComponent.h"

WaveformChartComponent::WaveformChartComponent() : fftLayer(AudioLayerManager<float>::getInstance().getFftDataLayer()), rmsDataLayer(AudioLayerManager<float>::getInstance().getRMSDataLayer()), truePeak(AudioLayerManager<float>::getInstance().getTruePeak())
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
}

WaveformChartComponent::~WaveformChartComponent()
{
    waveformChartCat.removeListener(this);
    fftLayer->setOnFftReadyCallback(nullptr);
}

void WaveformChartComponent::clear()
{
    DBG("waveformchart buffer clear.");
}

void WaveformChartComponent::pushStereoBuffer(const juce::AudioBuffer<float>* localAudioBuffer,const float localAudioBufferRMS)
{
    this->localAudioBuffer = localAudioBuffer;
    this->localAudioBufferRMS = localAudioBufferRMS;
}

int WaveformChartComponent::findStartNumByRisingEdge(const juce::AudioBuffer<float>& buffer, int channel)
{
    const int numSamples = buffer.getNumSamples();
    const float* data = buffer.getReadPointer(channel);

    int bestTriggerIndex = 0;
    float maxSlope = 0.0f;

    for (int i = 1; i < numSamples; ++i)
    {
        if (data[i - 1] < 0.0f && data[i] >= 0.0f)
        {
            float currentSlope = data[i] - data[i - 1];

            if (currentSlope > maxSlope)
            {
                maxSlope = currentSlope;
                bestTriggerIndex = i;
            }
        }
    }

    return (maxSlope > 0.001f) ? bestTriggerIndex : 0;
}

void WaveformChartComponent::paint(juce::Graphics& g)
{
    if (localAudioBuffer == nullptr) return;
    if (fftLayer->getAbsoluteMaxPeak().midIndex <= 0) return;
    const int numSamples = localAudioBuffer->getNumSamples();
    if (numSamples == 0) return;

    const int width = drawArea.getWidth();
    const int height = drawArea.getHeight();
    if (width <= 0 || height <= 0)
        return;

    const float midY = drawArea.getY() + height * 0.5f;
    const float leftX = static_cast<float>(drawArea.getX());

    if (currentMode == Autoalign)
    {
        int scopeSize = fftLayer->scopeSize;
        float freq = fftLayer->extractInterpolationFreq(fftLayer->getAbsoluteMaxPeak().lowIndex);
        scopeNum = 48000 / (freq > 0 ? freq : 1);
        if (scopeNum >= localAudioBuffer->getNumSamples()) scopeNum = localAudioBuffer->getNumSamples();
        startNum = findStartNumByRisingEdge(*localAudioBuffer, 0);
        if (startNum >= localAudioBuffer->getNumSamples()) startNum = 0;
        if (scopeNum + startNum > 1024) scopeNum = localAudioBuffer->getNumSamples() - startNum;
    }
    else if (currentMode == Normal)
    {
        startNum = 0;
        endNum = localAudioBuffer->getNumSamples();
        scopeNum = endNum - startNum;
    }

    for (int i = 0; i < localAudioBuffer->getNumChannels(); i++)
    {
        juce::Path waveformPath;
        juce::Path fillPath;

        waveformPath.preallocateSpace(scopeNum);
        fillPath.preallocateSpace(scopeNum + 2);

        std::vector<juce::Point<float>> points;
        points.reserve(scopeNum);
        bool firstPoint = true;
        const float* readPtr = localAudioBuffer->getReadPointer(i);
        for (int x = 0; x < scopeNum; ++x)
        {

            float y = midY - (readPtr[startNum + x] * (height * 0.5f));
            float screenX = leftX + ((float)x * width) / scopeNum;

            if (firstPoint)
            {
                waveformPath.startNewSubPath(screenX, y);
                firstPoint = false;
                points.emplace_back(screenX, y);
            }
            else
            {
                waveformPath.lineTo(screenX, y);
                points.emplace_back(screenX, y);
            }
        }
        if (i)
        {
            if (!points.empty())
            {
                fillPath.startNewSubPath(points[0]);
                for (size_t i = 1; i < points.size(); ++i)
                    fillPath.lineTo(points[i]);

                fillPath.lineTo(points.back().x, midY);
                fillPath.lineTo(leftX, midY);
                fillPath.closeSubPath();

                g.setColour(fillColorR.withAlpha(0.3f));
                g.fillPath(fillPath);
            }
            g.setColour(lineColorR.interpolatedWith(gradientColorOfLinesR, localAudioBufferRMS));
            g.strokePath(waveformPath, juce::PathStrokeType(2.0f));
        }
        else
        {
            if (!points.empty())
            {
                fillPath.startNewSubPath(points[0]);
                for (size_t i = 1; i < points.size(); ++i)
                    fillPath.lineTo(points[i]);

                fillPath.lineTo(points.back().x, midY);
                fillPath.lineTo(leftX, midY);
                fillPath.closeSubPath();

                g.setColour(fillColorL.withAlpha(0.3f));
                g.fillPath(fillPath);
            }
            g.setColour(lineColorL.interpolatedWith(gradientColorOfLinesL, localAudioBufferRMS));
            g.strokePath(waveformPath, juce::PathStrokeType(2.0f));
        }
    }
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