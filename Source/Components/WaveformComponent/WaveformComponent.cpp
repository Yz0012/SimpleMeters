#include "WaveformComponent.h"

WaveformComponent::WaveformComponent()
{
    CreateColoursConfiguration& createColoursConfiguration = CreateColoursConfiguration::getInstance();

    lineColorL = juce::Colour(createColoursConfiguration.colourHexToARGBInt(
        createColoursConfiguration.currentColourTheme.getChildWithProperty("name", "WaveformBoundaryLineL").getProperty("hex").toString(), true));
    fillColorL = juce::Colour(createColoursConfiguration.colourHexToARGBInt(
        createColoursConfiguration.currentColourTheme.getChildWithProperty("name", "WaveformFillL").getProperty("hex").toString(), true));
    gradientColorOfLinesL = juce::Colour(createColoursConfiguration.colourHexToARGBInt(
        createColoursConfiguration.currentColourTheme.getChildWithProperty("name", "WaveformGradientColorOfLinesL").getProperty("hex").toString(), true));
    lineColorR = juce::Colour(createColoursConfiguration.colourHexToARGBInt(
        createColoursConfiguration.currentColourTheme.getChildWithProperty("name", "WaveformBoundaryLineR").getProperty("hex").toString(), true));
    fillColorR = juce::Colour(createColoursConfiguration.colourHexToARGBInt(
        createColoursConfiguration.currentColourTheme.getChildWithProperty("name", "WaveformFillR").getProperty("hex").toString(), true));
    gradientColorOfLinesR = juce::Colour(createColoursConfiguration.colourHexToARGBInt(
        createColoursConfiguration.currentColourTheme.getChildWithProperty("name", "WaveformGradientColorOfLinesR").getProperty("hex").toString(), true));
}

WaveformComponent::~WaveformComponent()
{
	imageRingBuffer.reset();
}

void WaveformComponent::clear()
{
    DBG("waveformTile buffer clear.");
}

void WaveformComponent::drawWaveform(
    const juce::AudioBuffer<float>& localAudioBuffer,
    const float localAudioBufferRMS)
{
    const int width = tileArea.getWidth();
    const int height = tileArea.getHeight();
    if (width <= 0 || height <= 0)
        return;

    juce::Image tile(juce::Image::ARGB, tileArea.getWidth(), tileArea.getHeight(), true);
    juce::Graphics g(tile);

    const float midY = tileArea.getY() + height * 0.5f;
    const float leftX = static_cast<float>(tileArea.getX());

    for (int i = 0; i < localAudioBuffer.getNumChannels(); i++)
    {
        juce::Path waveformPath;
        bool firstPoint = true;
        const float* readPtr = localAudioBuffer.getReadPointer(i);
        for (int x = 0; x < localAudioBuffer.getNumSamples(); ++x)
        {

            float y = midY - (readPtr[x] * (height * 0.5f));
            float screenX = leftX + ((float)x * width) / localAudioBuffer.getNumSamples();

            if (firstPoint)
            {
                waveformPath.startNewSubPath(screenX, y);
                firstPoint = false;
            }
            else
            {
                waveformPath.lineTo(screenX, y);
            }
        }
        if (i)
        {
            g.setColour(lineColorR.interpolatedWith(gradientColorOfLinesR, localAudioBufferRMS));
            g.strokePath(waveformPath, juce::PathStrokeType(2.0f));
        }
        else
        {
            g.setColour(lineColorL.interpolatedWith(gradientColorOfLinesL, localAudioBufferRMS));
            g.strokePath(waveformPath, juce::PathStrokeType(2.0f));
        }
    }

    weakBuffer = imageRingBuffer;

    if (auto buffer = weakBuffer.lock())
    {
        imageRingBuffer->push(tile);
    }
    repaint();
}

void WaveformComponent::renderNextFrame(juce::Graphics& g, juce::Rectangle<int> bounds)
{
    if (bounds.getWidth() <= 0 || bounds.getHeight() <= 0)
        return;

    //drawAxis(g, bounds);

    //优化
    int tileSize = (int)(bounds.getWidth() / tileArea.getWidth()) + 1;
    if (this->tileSize != tileSize)
    {
		imageRingBuffer = std::make_shared<CircularImageBuffer>(tileSize);
        this->tileSize = tileSize;
    }

	if (!imageRingBuffer) return;

	weakBuffer = imageRingBuffer;

    for (int i = 0; i < imageRingBuffer->size(); i++)
    {
        if (auto buffer = weakBuffer.lock())
        {
            int actualIndex = (buffer->getWriteIndex() + i) % buffer->size();
            juce::Image imageTile = buffer->get(actualIndex);
            if (imageTile.isValid()) {
                //
                g.drawImageAt(imageTile, bounds.getX() + i * (tileArea.getWidth() - 1), bounds.getY());
            }
        }
    }
}

void WaveformComponent::drawAxis(juce::Graphics& g, juce::Rectangle<int> area)
{
    g.setColour(juce::Colours::blueviolet.withAlpha(0.5f));
    g.setFont(10.0f);

    const float topY = (float)area.getY();
    const float bottomY = (float)(area.getY() + area.getHeight());
    const float midY = (topY + bottomY) * 0.5f;
    const float halfHeight = (float)area.getHeight() * 0.5f;

    const int labelX = area.getX() - 40;
    const int labelW = 40;

    {
        g.drawText("0 dBFS", labelX, (int)topY - 10, labelW, 20,
            juce::Justification::centredRight);

        juce::Path p;
        p.startNewSubPath((float)area.getX(), topY);
        p.lineTo((float)(area.getX() + area.getWidth()), topY);
        g.strokePath(p, juce::PathStrokeType(1.0f));
    }

    for (float db : dbValues)
    {
        const float amplitude = std::pow(10.0f, db / 20.0f);
        const float yTop = midY - halfHeight * amplitude;

        {
            juce::Path line;
            line.startNewSubPath((float)area.getX(), yTop);
            line.lineTo((float)(area.getX() + area.getWidth()), yTop);
            g.strokePath(line, juce::PathStrokeType(1.0f));
        }

        juce::String label = juce::String((int)db) + " dBFS";
        g.drawText(label, labelX, (int)yTop - 10, labelW, 20,
            juce::Justification::centredRight);
    }
    for (float db : dbValues)
    {
        const float amplitude = std::pow(10.0f, db / 20.0f);
        const float yTop = midY + halfHeight * amplitude;

        {
            juce::Path line;
            line.startNewSubPath((float)area.getX(), yTop);
            line.lineTo((float)(area.getX() + area.getWidth()), yTop);
            g.strokePath(line, juce::PathStrokeType(1.0f));
        }

        juce::String label = juce::String((int)db) + " dBFS";
        g.drawText(label, labelX, (int)yTop - 10, labelW, 20,
            juce::Justification::centredRight);
    }

    {
        g.drawText("0 dBFS", labelX, (int)topY - 10, labelW, 20,
            juce::Justification::centredRight);

        juce::Path p;
        p.startNewSubPath((float)area.getX(), bottomY);
        p.lineTo((float)(area.getX() + area.getWidth()), bottomY);
        g.strokePath(p, juce::PathStrokeType(1.0f));
    }
}

void WaveformComponent::paint(juce::Graphics& g)
{
    renderNextFrame(g, drawArea);
}