
#include "SpectrumAnalyser.h"

SpectrumAnalyser::SpectrumAnalyser() : forwardFFT(fftOrder), window(fftSize, juce::dsp::WindowingFunction<float>::hann)
{
    CreateColoursConfiguration& createColoursConfiguration = CreateColoursConfiguration::getInstance();

    spectrumCat = createColoursConfiguration.currentColourTheme
        .getChildWithProperty("name", "SpectrumAnalyzer");
    lineColor = juce::Colour(createColoursConfiguration.colourHexToARGBInt(
        spectrumCat.getChildWithProperty("name", "BoundaryLine").getProperty("hex").toString(), false));
    fillColor = juce::Colour(createColoursConfiguration.colourHexToARGBInt(
        spectrumCat.getChildWithProperty("name", "Fill").getProperty("hex").toString(), false));

    for (int i = 0; i < scopeSize; ++i)
    {
        scopeData[i] = mindB;
        gapSmoothedScopeData[i] = mindB;
        scopeDataStorage[i] = mindB;

        scopeData2[i] = mindB;
        gapSmoothedScopeData2[i] = mindB;
        scopeDataStorage2[i] = mindB;
    }

    lastProcessTime = juce::Time::getMillisecondCounterHiRes();

    addAndMakeVisible(&drawBounds);
    addAndMakeVisible(&componentHeader);

    spectrumCat.addListener(this);
}

SpectrumAnalyser::~SpectrumAnalyser()
{
    spectrumCat.removeListener(this);
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

    if (currentMode == Left)
    {
        const int channelsToMix = juce::jmin(buffer.getNumChannels(), 2);
        for (int i = 0; i < numSamples; ++i)
        {
            pushNextSampleIntoFifo(buffer.getReadPointer(0)[i], 0);
        }
    }
    else if (currentMode == Right)
    {
        const int channelsToMix = juce::jmin(buffer.getNumChannels(), 2);
        for (int i = 0; i < numSamples; ++i)
        {
            pushNextSampleIntoFifo(buffer.getReadPointer(1)[i], 0);
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
    if (currentMode == Left || currentMode == Right || currentMode == Interleaved)
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
    const float width = static_cast<float>(bounds.getWidth());
    const float bottom = static_cast<float>(bounds.getBottom());
    const float top = static_cast<float>(bounds.getY());
    const float leftX = static_cast<float>(bounds.getX());

    for (int i = 0; i < scopeSize; ++i)
    {
        gapSmoothedScopeData[i] = (scopeData[i] - scopeDataStorage[i]) * 0.70f;
        scopeDataStorage[i] = scopeData[i] - gapSmoothedScopeData[i];
    }

    const int numBands = 256;
    std::vector<juce::Point<float>> points;
    points.reserve(numBands);

    points.emplace_back(leftX,
        juce::jmap(scopeDataStorage[0], mindB, maxdB, bottom, top));

    for (int band = 0; band < numBands; ++band)
    {
        const int fStart = band * scopeSize / numBands;
        const int fEnd = (band + 1) * scopeSize / numBands;

        float maxVal = scopeDataStorage[fStart];
        for (int i = fStart + 1; i < fEnd; ++i)
        {
            if (scopeDataStorage[i] > maxVal)
                maxVal = scopeDataStorage[i];
        }

        const float midIdx = (fStart + fEnd - 1) * 0.5f;

        const float f = std::log(midIdx + std::exp(1.0f)) - 1.0f;
        const float x = (f / scopeSizeTransformed) * width + leftX;

        const float y = juce::jmap(maxVal, mindB, maxdB, bottom, top);

        points.emplace_back(x, y);
    }

    if (points.size() < 2)
        return;

    juce::Path linePath;
    linePath.startNewSubPath(points[0]);

    for (size_t i = 0; i < points.size() - 1; ++i)
    {
        const juce::Point<float> p1 = points[i];
        const juce::Point<float> p2 = points[i + 1];

        const juce::Point<float> p0 = (i == 0) ? (p1 - (p2 - p1)) : points[i - 1];
        const juce::Point<float> p3 = (i + 2 >= points.size()) ? (p2 + (p2 - p1)) : points[i + 2];

        const auto cp1 = p1 + (p2 - p0) / 6.0f;
        const auto cp2 = p2 + (p1 - p3) / 6.0f;

        linePath.cubicTo(cp1, cp2, p2);
    }

    juce::Path fillPath = linePath;
    fillPath.lineTo(points.back().x, bottom);
    fillPath.lineTo(leftX, bottom);
    fillPath.closeSubPath();

    g.setGradientFill(juce::ColourGradient::vertical(
        fillColour.withAlpha(0.9f), top,
        fillColour.withAlpha(0.3f), bottom));
    g.fillPath(fillPath);

    g.setColour(lineColour);
    g.strokePath(linePath, juce::PathStrokeType(1.0f));
}

void SpectrumAnalyser::paint(juce::Graphics& g)
{
    drawFrame(g, drawArea);
}

void SpectrumAnalyser::timerCallback()
{
    checkProcessBufferActivity();
	repaint(getLocalBounds());
}

void SpectrumAnalyser::callStopTimer()
{
    stopTimer();
}

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

		scopeData2[i] = mindB;
		gapSmoothedScopeData2[i] = mindB;
		scopeDataStorage2[i] = mindB;
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

void SpectrumAnalyser::mouseDown(const juce::MouseEvent& event)
{
    if (event.mods.isPopupMenu())
    {
        if (cb == nullptr) return;
        cb();
    }
}

void SpectrumAnalyser::mouseEnter(const juce::MouseEvent& event)
{
    drawBounds.setVisible(true);
    componentHeader.setVisible(true);
}

void SpectrumAnalyser::mouseExit(const juce::MouseEvent&)
{
    if (!componentHeader.isMouseOver())
    {
        drawBounds.setVisible(false);
        componentHeader.setVisible(false);
    }
}

void SpectrumAnalyser::valueTreePropertyChanged(juce::ValueTree& tree, const juce::Identifier& property)
{
    if (property == juce::Identifier("hex"))
    {
        this->lineColor = juce::Colour(CreateColoursConfiguration::getInstance().colourHexToARGBInt(
            spectrumCat.getChildWithProperty("name", "BoundaryLine").getProperty("hex").toString(), false));
		this->fillColor = juce::Colour(CreateColoursConfiguration::getInstance().colourHexToARGBInt(
			spectrumCat.getChildWithProperty("name", "Fill").getProperty("hex").toString(), false));
        repaint();
    }
}