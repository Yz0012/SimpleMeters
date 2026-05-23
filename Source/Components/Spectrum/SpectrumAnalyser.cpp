
#include "SpectrumAnalyser.h"

SpectrumAnalyser::SpectrumAnalyser() : forwardFFT(fftOrder), window(fftSize, juce::dsp::WindowingFunction<float>::hann)
{
    CreateColoursConfiguration& createColoursConfiguration = CreateColoursConfiguration::getInstance();

    lineColor = juce::Colour(createColoursConfiguration.colourHexToARGBInt(
        createColoursConfiguration.currentColourTheme.getChildWithProperty("name", "SpectrumAnalyzerBoundaryLine").getProperty("hex").toString(), false));
    fillColor = juce::Colour(createColoursConfiguration.colourHexToARGBInt(
        createColoursConfiguration.currentColourTheme.getChildWithProperty("name", "SpectrumAnalyzerFill").getProperty("hex").toString(), false));

    for (int i = 0; i < scopeSize; ++i)
    {
        scopeData[i] = mindB;
        gapSmoothedScopeData[i] = mindB;
        scopeDataStorage[i] = mindB;
    }

    lastProcessTime = juce::Time::getMillisecondCounterHiRes();
}

SpectrumAnalyser::~SpectrumAnalyser()
{
}

void SpectrumAnalyser::processAudioBuffer(const juce::AudioBuffer<float>& buffer)
{
    lastProcessTime = juce::Time::getMillisecondCounterHiRes();

    if (!isTimerRunning())
    {
        startTimerHz(60);
    }

    const int numSamples = buffer.getNumSamples();
    if (numSamples == 0) return;

    const int numChannels = buffer.getNumChannels();

    if (currentMode == Mono)
    {
        const int channelsToMix = juce::jmin(buffer.getNumChannels(), 2);
        for (int i = 0; i < numSamples; ++i)
        {
            float mixed = 0.0f;
            for (int ch = 0; ch < channelsToMix; ++ch)
                mixed += buffer.getReadPointer(ch)[i];
            mixed /= static_cast<float>(channelsToMix);
            pushNextSampleIntoFifo(mixed, 0);
        }
    }
    else if (currentMode == Stereo && numChannels >= 2)
    {
        const float* left = buffer.getReadPointer(0);
        const float* right = buffer.getReadPointer(1);
        for (int i = 0; i < numSamples; ++i)
        {
            pushNextSampleIntoFifo(left[i], 0);
            pushNextSampleIntoFifo(right[i], 1);
        }
    }
    else if (currentMode == LR && numChannels >= 2)
    {
        const float* left = buffer.getReadPointer(0);
        const float* right = buffer.getReadPointer(1);
        for (int i = 0; i < numSamples; ++i)
        {
            float lrSample = left[i] - right[i];
            pushNextSampleIntoFifo(left[i], 0);
            pushNextSampleIntoFifo(lrSample, 1);
        }
    }
    else if (currentMode == Interleaved && numChannels >= 2)
    {
        const float* left = buffer.getReadPointer(0);
        const float* right = buffer.getReadPointer(1);
        for (int i = 0; i < numSamples; ++i)
        {
            pushNextSampleIntoFifo(left[i], 0);
            pushNextSampleIntoFifo(right[i], 0);
        }
    }
}

void SpectrumAnalyser::pushNextSampleIntoFifo(float sample, int channelIndex) noexcept
{
    if (channelIndex == 0)
    {
        if (fifoIndex == fftSize)
        {
            if (!nextFFTBlockReady)
            {
                juce::zeromem(fftData, sizeof(fftData));
                memcpy(fftData, fifo, sizeof(fifo));
                nextFFTBlockReady = true;
                drawNextFrameOfSpectrum(0);
            }
            fifoIndex = 0;
        }
        fifo[fifoIndex++] = sample;
    }
    else
    {
        if (fifoIndex2 == fftSize)
        {
            if (!nextFFTBlockReady2)
            {
                juce::zeromem(fftData2, sizeof(fftData2));
                memcpy(fftData2, fifo2, sizeof(fifo2));
                nextFFTBlockReady2 = true;
                drawNextFrameOfSpectrum(1);
            }
            fifoIndex2 = 0;
        }
        fifo2[fifoIndex2++] = sample;
    }
}

void SpectrumAnalyser::drawNextFrameOfSpectrum(int channelIndex)
{
    float* fftBlock = (channelIndex == 0) ? fftData : fftData2;
    float* destScope = (channelIndex == 0) ? scopeData : scopeData2;

    window.multiplyWithWindowingTable(fftBlock, fftSize);
    forwardFFT.performFrequencyOnlyForwardTransform(fftBlock);

    if (currentMode == Interleaved)
    {
        for (int i = 0; i < scopeSize; ++i)
        {
            auto skewedProportionX = i / (float)scopeSize;
            auto fftDataIndex = juce::jlimit(0, fftSize / 2, (int)(skewedProportionX * (float)fftSize * 0.25f));
            auto sourceDecibels = juce::Decibels::gainToDecibels(fftBlock[fftDataIndex]) - juce::Decibels::gainToDecibels((float)fftSize);
            destScope[i] = juce::jlimit(mindB, maxdB, sourceDecibels + (2.0f * std::log(1.0f * i + std::exp(1.0f)) - 7) * 3.0f);
        }
    }
    else
    {
        for (int i = 0; i < scopeSize; ++i)
        {
            auto skewedProportionX = i / (float)scopeSize;
            auto fftDataIndex = juce::jlimit(0, fftSize / 2, (int)(skewedProportionX * (float)fftSize * 0.5f));
            auto sourceDecibels = juce::Decibels::gainToDecibels(fftBlock[fftDataIndex]) - juce::Decibels::gainToDecibels((float)fftSize);
            destScope[i] = juce::jlimit(mindB, maxdB, sourceDecibels + (2.0f * std::log(1.0f * i + std::exp(1.0f)) - 7) * 3.0f);
        }
    }

    if (channelIndex == 0)
        nextFFTBlockReady = false;
    else
        nextFFTBlockReady2 = false;
}

void SpectrumAnalyser::drawFrame(juce::Graphics& g, juce::Rectangle<int> bounds)
{
    if (currentMode == Mono || currentMode == Interleaved)
    {
        drawSingleCurve(g, bounds, scopeData, scopeDataStorage, gapSmoothedScopeData,
            lineColor, fillColor);
    }
    else if (currentMode == LR)
    {
        juce::Colour line2 = juce::Colours::orange;
        juce::Colour fill2 = line2.withAlpha(0.3f);
        drawSingleCurve(g, bounds, scopeData2, scopeDataStorage2, gapSmoothedScopeData2,
            line2, fill2);
    }
    else if (currentMode == Stereo)
    {
        drawSingleCurve(g, bounds, scopeData, scopeDataStorage, gapSmoothedScopeData,
            lineColor, fillColor);
        juce::Colour line2 = juce::Colours::greenyellow;
        juce::Colour fill2 = line2.withAlpha(0.3f);
        drawSingleCurve(g, bounds, scopeData2, scopeDataStorage2, gapSmoothedScopeData2,
            line2, fill2);
    }

}

void SpectrumAnalyser::drawSingleCurve(juce::Graphics& g,
    juce::Rectangle<int> bounds,
    const float* scopeData,
    float* scopeDataStorage,
    float* gapSmoothedScopeData,
    juce::Colour lineColour,
    juce::Colour fillColour)
{
    juce::Path linePath;
    juce::Path fillPath;
    std::vector<juce::Point<float>> points;
    points.reserve(scopeSize);

    bool firstPoint = true;
    const float width = static_cast<float>(bounds.getWidth());
    const float bottom = static_cast<float>(bounds.getBottom());
    const float top = static_cast<float>(bounds.getY());

    for (int i = 0; i < scopeSize; ++i)
    {
        const float f = std::log(static_cast<float>(i) + std::exp(1.0f)) - 1.0f;
        const float currentNum = (f / scopeSizeTransformed) * width + bounds.getX();

        if (firstPoint)
        {
            float y = juce::jmap(scopeDataStorage[i], mindB, maxdB, bottom, top);
            linePath.startNewSubPath(bounds.getX(), y);
            firstPoint = false;

            gapSmoothedScopeData[i] = (scopeData[i] - scopeDataStorage[i]) * 0.70f;
            scopeDataStorage[i] = scopeData[i] - gapSmoothedScopeData[i];

            points.emplace_back(bounds.getX(), y);
        }
        else
        {
            float y = juce::jmap(scopeDataStorage[i], mindB, maxdB, bottom, top);
            linePath.lineTo(currentNum, y);

            gapSmoothedScopeData[i] = (scopeData[i] - scopeDataStorage[i]) * 0.70f;
            scopeDataStorage[i] = scopeData[i] - gapSmoothedScopeData[i];

            points.emplace_back(currentNum, y);
        }
    }

    if (!points.empty())
    {
        fillPath.startNewSubPath(points[0]);
        for (size_t i = 1; i < points.size(); ++i)
            fillPath.lineTo(points[i]);

        fillPath.lineTo(points.back().x, bottom);
        fillPath.lineTo(bounds.getX(), bottom);
        fillPath.closeSubPath();

        g.setGradientFill(juce::ColourGradient::vertical(
            fillColour.withAlpha(0.9f), top,
            fillColour.withAlpha(0.3f), bottom));
        g.fillPath(fillPath);
    }

    g.setColour(lineColour);
    g.strokePath(linePath, juce::PathStrokeType(1.0f));
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
    checkProcessBufferActivity();
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

void SpectrumAnalyser::scopeDataReset()
{
    for (int i = 0; i < scopeSize; ++i)
    {
        scopeData[i] = mindB;
        gapSmoothedScopeData[i] = mindB;
        scopeDataStorage[i] = mindB;
    }
}

void SpectrumAnalyser::setAnalysisMode(AnalysisMode mode)
{
    if (currentMode != mode)
    {
        currentMode = mode;
        scopeDataReset();
        fifoIndex = 0;
        fifoIndex2 = 0;
        nextFFTBlockReady = false;
        nextFFTBlockReady2 = false;
    }
}

void SpectrumAnalyser::checkProcessBufferActivity()
{
    auto now = juce::Time::getMillisecondCounterHiRes();
    if (now - lastProcessTime > 2000.0)
    {
        stopTimer();
    }
}