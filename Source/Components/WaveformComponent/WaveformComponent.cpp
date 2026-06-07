#include "WaveformComponent.h"

WaveformComponent::WaveformComponent()
{
    CreateColoursConfiguration& createColoursConfiguration = CreateColoursConfiguration::getInstance();

    waveformCat = createColoursConfiguration.currentColourTheme
        .getChildWithProperty("name", "Waveform");
    lineColorL = juce::Colour(createColoursConfiguration.colourHexToARGBInt(
        waveformCat.getChildWithProperty("name", "BoundaryLineL").getProperty("hex").toString(), true));
    fillColorL = juce::Colour(createColoursConfiguration.colourHexToARGBInt(
        waveformCat.getChildWithProperty("name", "FillL").getProperty("hex").toString(), true));
    gradientColorOfLinesL = juce::Colour(createColoursConfiguration.colourHexToARGBInt(
        waveformCat.getChildWithProperty("name", "GradientColorOfLinesL").getProperty("hex").toString(), true));
    lineColorR = juce::Colour(createColoursConfiguration.colourHexToARGBInt(
        waveformCat.getChildWithProperty("name", "BoundaryLineR").getProperty("hex").toString(), true));
    fillColorR = juce::Colour(createColoursConfiguration.colourHexToARGBInt(
        waveformCat.getChildWithProperty("name", "FillR").getProperty("hex").toString(), true));
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
        waveformPath.preallocateSpace(localAudioBuffer.getNumSamples());
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
}

void WaveformComponent::renderNextFrame(juce::Graphics& g, juce::Rectangle<int> bounds)
{
    if (bounds.getWidth() <= 0 || bounds.getHeight() <= 0)
        return;

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

void WaveformComponent::paint(juce::Graphics&  g)
{
    renderNextFrame(g, drawArea);
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

void WaveformComponent::valueTreePropertyChanged(juce::ValueTree& tree, const juce::Identifier& property)
{
    if (property == juce::Identifier("hex"))
    {
        lineColorL = juce::Colour(CreateColoursConfiguration::getInstance().colourHexToARGBInt(
            waveformCat.getChildWithProperty("name", "BoundaryLineL").getProperty("hex").toString(), true));
        fillColorL = juce::Colour(CreateColoursConfiguration::getInstance().colourHexToARGBInt(
            waveformCat.getChildWithProperty("name", "FillL").getProperty("hex").toString(), true));
        gradientColorOfLinesL = juce::Colour(CreateColoursConfiguration::getInstance().colourHexToARGBInt(
            waveformCat.getChildWithProperty("name", "GradientColorOfLinesL").getProperty("hex").toString(), true));
        lineColorR = juce::Colour(CreateColoursConfiguration::getInstance().colourHexToARGBInt(
            waveformCat.getChildWithProperty("name", "BoundaryLineR").getProperty("hex").toString(), true));
        fillColorR = juce::Colour(CreateColoursConfiguration::getInstance().colourHexToARGBInt(
            waveformCat.getChildWithProperty("name", "FillR").getProperty("hex").toString(), true));
        gradientColorOfLinesR = juce::Colour(CreateColoursConfiguration::getInstance().colourHexToARGBInt(
            waveformCat.getChildWithProperty("name", "GradientColorOfLinesR").getProperty("hex").toString(), true));
        repaint();
    }
}