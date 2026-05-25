
#include "WaveformChartComponent.h"

WaveformChartComponent::WaveformChartComponent()
{
    CreateColoursConfiguration& createColoursConfiguration = CreateColoursConfiguration::getInstance();

    lineColorL = juce::Colour(createColoursConfiguration.colourHexToARGBInt(
        createColoursConfiguration.currentColourTheme.getChildWithProperty("name", "WaveformChartBoundaryLineL").getProperty("hex").toString(), true));
    fillColorL = juce::Colour(createColoursConfiguration.colourHexToARGBInt(
        createColoursConfiguration.currentColourTheme.getChildWithProperty("name", "WaveformChartFillL").getProperty("hex").toString(), true));
    gradientColorOfLinesL = juce::Colour(createColoursConfiguration.colourHexToARGBInt(
        createColoursConfiguration.currentColourTheme.getChildWithProperty("name", "WaveformChartGradientColorOfLinesL").getProperty("hex").toString(), true));
    lineColorR = juce::Colour(createColoursConfiguration.colourHexToARGBInt(
        createColoursConfiguration.currentColourTheme.getChildWithProperty("name", "WaveformChartBoundaryLineR").getProperty("hex").toString(), true));
    fillColorR = juce::Colour(createColoursConfiguration.colourHexToARGBInt(
        createColoursConfiguration.currentColourTheme.getChildWithProperty("name", "WaveformChartFillR").getProperty("hex").toString(), true));
    gradientColorOfLinesR = juce::Colour(createColoursConfiguration.colourHexToARGBInt(
        createColoursConfiguration.currentColourTheme.getChildWithProperty("name", "WaveformChartGradientColorOfLinesR").getProperty("hex").toString(), true));

    addAndMakeVisible(&componentControl);
}

WaveformChartComponent::~WaveformChartComponent()
{
}

void WaveformChartComponent::clear()
{
    DBG("waveformchart buffer clear.");
}

void WaveformChartComponent::pushStereoBuffer(const juce::AudioBuffer<float>* localAudioBuffer,const float* localAudioBufferRMS)
{
    this->localAudioBuffer = localAudioBuffer;
    this->localAudioBufferRMS = localAudioBufferRMS;
}

void WaveformChartComponent::paint(juce::Graphics& g)
{
    if (localAudioBuffer == nullptr) return;
    if (localAudioBufferRMS == nullptr) return;
    const int numSamples = localAudioBuffer->getNumSamples();
    if (numSamples == 0) return;

    const int width = drawArea.getWidth();
    const int height = drawArea.getHeight();
    if (width <= 0 || height <= 0)
        return;

    const float midY = drawArea.getY() + height * 0.5f;
    const float leftX = static_cast<float>(drawArea.getX());

    for (int i = 0; i < localAudioBuffer->getNumChannels(); i++)
    {
        juce::Path waveformPath;
        juce::Path fillPath;

        waveformPath.preallocateSpace(numSamples);
        fillPath.preallocateSpace(numSamples + 2);

        std::vector<juce::Point<float>> points;
        points.reserve(numSamples);
        bool firstPoint = true;
        const float* readPtr = localAudioBuffer->getReadPointer(i);
        for (int x = 0; x < numSamples; ++x)
        {

            float y = midY - (readPtr[x] * (height * 0.5f));
            float screenX = leftX + ((float)x * width) / numSamples;

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
            g.setColour(lineColorR.interpolatedWith(gradientColorOfLinesR, *localAudioBufferRMS));
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
            g.setColour(lineColorL.interpolatedWith(gradientColorOfLinesL, *localAudioBufferRMS));
            g.strokePath(waveformPath, juce::PathStrokeType(2.0f));
        }
    }
}

void WaveformChartComponent::mouseEnter(const juce::MouseEvent& event)
{
    componentControl.setVisible(true);
}

void WaveformChartComponent::mouseExit(const juce::MouseEvent&)
{
    if (!componentControl.isMouseOver())
    {
        componentControl.setVisible(false);
    }
}