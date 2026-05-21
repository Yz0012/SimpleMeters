
#include "SpectrumAnalyser.h"

SpectrumAnalyser::SpectrumAnalyser() : forwardFFT(fftOrder), window(fftSize, juce::dsp::WindowingFunction<float>::hann)
{
    createColoursConfiguration = CreateColoursConfiguration::getInstance();

    lineColor = juce::Colour(createColoursConfiguration->colourHexToARGBInt(
        createColoursConfiguration->currentColourTheme.getChildWithProperty("name", "SpectrumAnalyzerBoundaryLine").getProperty("hex").toString(), false));
    fillColor = juce::Colour(createColoursConfiguration->colourHexToARGBInt(
        createColoursConfiguration->currentColourTheme.getChildWithProperty("name", "SpectrumAnalyzerFill").getProperty("hex").toString(), false));

    for (int i = 0; i < scopeSize; ++i)
    {
        scopeData[i] = mindB;
        gapSmoothedScopeData[i] = mindB;
        scopeDataStorage[i] = mindB;
    }

    startTimerHz(60);
}

SpectrumAnalyser::~SpectrumAnalyser()
{
}

void SpectrumAnalyser::pushNextSampleIntoFifo(float sample) noexcept
{
    if (fifoIndex == fftSize)
    {
        if (!nextFFTBlockReady)
        {
            juce::zeromem(fftData, sizeof(fftData));
            memcpy(fftData, fifo, sizeof(fifo));
            nextFFTBlockReady = true;
            drawNextFrameOfSpectrum();
        }
        fifoIndex = 0;
    }
    fifo[fifoIndex++] = sample;
}

void SpectrumAnalyser::drawNextFrameOfSpectrum()
{
    window.multiplyWithWindowingTable(fftData, fftSize);
    forwardFFT.performFrequencyOnlyForwardTransform(fftData);

    for (int i = 0; i < scopeSize; ++i)
    {   
        auto skewedProportionX = i / (float)scopeSize;
        auto fftDataIndex = juce::jlimit(0, fftSize / 2, (int)(skewedProportionX * (float)fftSize * 0.25f));
        auto sourceDecibels = juce::Decibels::gainToDecibels(fftData[fftDataIndex]) - juce::Decibels::gainToDecibels((float)fftSize);
        level = juce::jlimit(mindB, maxdB, sourceDecibels + (2.0f * std::log(1.0f * i + std::exp(1.0f)) - 7) * 3.0f);
        scopeData[i] = level;
    }
    nextFFTBlockReady = false;
}

void SpectrumAnalyser::drawFrame(juce::Graphics& g,juce::Rectangle<int> bounds)
{
    juce::Path p;
    juce::Path fillPath;
    bool firstPoint = true;

    std::vector<juce::Point<float>> points;
    points.reserve(bounds.getWidth());

    //drawFrequencyAxis(g, bounds);

    for (int i = 0; i < scopeSize; ++i)
    {
        float f = std::log((float)i + std::exp(1.0f)) - 1;
        float width = bounds.getWidth();
        float bottom = bounds.getBottom();
		float initialY = bounds.getY();
        float currentNum = (float)f / scopeSizeTransformed * width + bounds.getX();
        if (firstPoint)
        {
            float y = juce::jmap(scopeDataStorage[i], mindB, maxdB, bottom, initialY);
            p.startNewSubPath(bounds.getX(), y);
            firstPoint = false;
            gapSmoothedScopeData[i] = (scopeData[i] - scopeDataStorage[i]) * 0.50f;
            scopeDataStorage[i] = scopeData[i] - gapSmoothedScopeData[i];
            points.emplace_back(bounds.getX(), y);
        }
        else
        {
            float y = juce::jmap(scopeDataStorage[i], mindB, maxdB, bottom, initialY);
            p.lineTo(currentNum, y);
            gapSmoothedScopeData[i] = (scopeData[i] - scopeDataStorage[i]) * 0.50f;
            scopeDataStorage[i] = scopeData[i] - gapSmoothedScopeData[i];
            points.emplace_back(currentNum, y);
        }
    }

    // 构建填充路径：波形曲线 + 沿中线返回
    if (!points.empty())
    {
        fillPath.startNewSubPath(bounds.getX(), points[0].y);
        for (size_t i = 1; i < points.size(); ++i)
            fillPath.lineTo(points[i]);
        // 从最后一个点沿底面水平回到第一个点
        fillPath.lineTo(points[scopeSize - 1].x, bounds.getBottom());
        fillPath.lineTo(bounds.getX(), bounds.getBottom());
        fillPath.closeSubPath();

        g.setGradientFill(juce::ColourGradient::vertical(fillColor.withAlpha(0.9f), bounds.getY(), fillColor.withAlpha(0.3f), bounds.getBottom()));
        g.fillPath(fillPath);
    }

    g.setColour(lineColor);
    g.strokePath(p, juce::PathStrokeType(1.0f));
}

void SpectrumAnalyser::drawFrequencyAxis(juce::Graphics& g,juce::Rectangle<int> bounds)
{
    juce::Path p;
    g.setColour(juce::Colours::blueviolet.withAlpha(0.2f));
    g.setFont(10.0f);
    float width = bounds.getWidth();
    float currentNum;
	float previousNum = 0.0f;
    for (int i = 0; i < 10; ++i)
    {
        float f = std::log((float)i + std::exp(1.0f)) - 1;
        currentNum = (float)f / (std::log((float)10 + std::exp(1.0f)) - 1) * width / 3.0f + bounds.getX();
        p.startNewSubPath(currentNum, bounds.getBottom());
        p.lineTo(currentNum, bounds.getY());
    }
	previousNum = width / 3.0f;
    for (int i = 0; i < 10; ++i)
    {
        float f = std::log((float)i + std::exp(1.0f)) - 1;
        currentNum = (float)f / (std::log((float)10 + std::exp(1.0f)) - 1) * width / 3.0f + bounds.getX();
        p.startNewSubPath(currentNum + previousNum, bounds.getBottom());
        p.lineTo(currentNum + previousNum, bounds.getY());
    }
	previousNum = width / 3.0f * 2;
    for (int i = 0; i < 10; ++i)
    {
        float f = std::log((float)i + std::exp(1.0f)) - 1;
        currentNum = (float)f / (std::log((float)10 + std::exp(1.0f)) - 1) * width / 3.0f + bounds.getX();
        p.startNewSubPath(currentNum + previousNum, bounds.getBottom());
        p.lineTo(currentNum + previousNum, bounds.getY());
    }
    g.strokePath(p, juce::PathStrokeType(1.0f));
}

void SpectrumAnalyser::paint(juce::Graphics& g)
{
    drawFrame(g, drawArea);
}

void SpectrumAnalyser::timerCallback()
{
	repaint();
}

void SpectrumAnalyser::callStopTimer()
{
    stopTimer();
}

/*
* Control the refresh rate of the component
* @param RefreshRate
*/
void SpectrumAnalyser::callstartTimerHz(int hz)
{
    startTimerHz(hz);
}