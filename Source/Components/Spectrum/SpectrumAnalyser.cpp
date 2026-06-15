
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
        scopeDataStorage[i] = mindB;

        scopeData2[i] = mindB;
        scopeDataStorage2[i] = mindB;

        destScope1[i] = mindB;
        destScope2[i] = mindB;
    }

    lastProcessTime = juce::Time::getMillisecondCounterHiRes();

    addAndMakeVisible(&drawBounds);
    addAndMakeVisible(&componentHeader);
	addAndMakeVisible(&eqReferenceLines);

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
        for (int i = 0; i < numSamples; ++i)
        {
            pushNextSampleIntoFifo(buffer.getReadPointer(0)[i], 0);
        }
    }
    else if (currentMode == Right)
    {
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
    std::array<float, scopeSize>& destScope = (channelIndex == 0) ? scopeData : scopeData2;
    std::array<float, scopeSize>& destScopeGain = (channelIndex == 0) ? destScope1 : destScope2;

    window.multiplyWithWindowingTable(fftBlock, fftSize);
    forwardFFT.performFrequencyOnlyForwardTransform(fftBlock);

    if (currentMode == Interleaved)
    {
        for (int i = 0; i < scopeSize; ++i)
        {
            auto skewedProportionX = i / (float)scopeSize;
            auto fftDataIndex = juce::jlimit(0, fftSize / 2, (int)(skewedProportionX * (float)fftSize * 0.25f));
            auto sourceDecibels = juce::Decibels::gainToDecibels(fftBlock[fftDataIndex]) - juce::Decibels::gainToDecibels((float)fftSize);
            destScope[i] = sourceDecibels;
            destScopeGain[i] = juce::jlimit(mindB, maxdB, sourceDecibels + (2.0f * std::log(1.0f * i + std::exp(1.0f)) - 7) * 3.0f);
        }
    }
    else
    {
        for (int i = 0; i < scopeSize; ++i)
        {
            auto skewedProportionX = i / (float)scopeSize;
            auto fftDataIndex = juce::jlimit(0, fftSize / 2, (int)(skewedProportionX * (float)fftSize * 0.5f));
            auto sourceDecibels = juce::Decibels::gainToDecibels(fftBlock[fftDataIndex]) - juce::Decibels::gainToDecibels((float)fftSize);
            destScope[i] = sourceDecibels;
            destScopeGain[i] = juce::jlimit(mindB, maxdB, sourceDecibels + (2.0f * std::log(1.0f * i + std::exp(1.0f)) - 7) * 3.0f);
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
        drawSingleCurve(g, bounds, scopeData, scopeDataStorage, destScope1,
            lineColor, fillColor);
    }
    else if (currentMode == LR)
    {
        juce::Colour line2 = juce::Colours::orange;
        juce::Colour fill2 = line2.withAlpha(0.3f);
        drawSingleCurve(g, bounds, scopeData2, scopeDataStorage2, destScope2,
            line2, fill2);
    }
    else if (currentMode == Stereo)
    {
        drawSingleCurve(g, bounds, scopeData, scopeDataStorage, destScope1,
            lineColor, fillColor);
        juce::Colour line2 = juce::Colours::greenyellow;
        juce::Colour fill2 = line2.withAlpha(0.3f);
        drawSingleCurve(g, bounds, scopeData2, scopeDataStorage2, destScope2,
            line2, fill2);
    }

}

void SpectrumAnalyser::drawSingleCurve(juce::Graphics& g,
    juce::Rectangle<int> bounds,
    const std::array<float, scopeSize>& scopeData,
    std::array<float, scopeSize>& scopeDataStorage,
    std::array<float, scopeSize>& destScope,
    juce::Colour lineColour,
    juce::Colour fillColour)
{
    const float width = static_cast<float>(bounds.getWidth());
    const float bottom = static_cast<float>(bounds.getBottom());
    const float top = static_cast<float>(bounds.getY());
    const float leftX = static_cast<float>(bounds.getX());

    float globalMaxVal = -std::numeric_limits<float>::max();
    int globalMaxBinIdx = 0;
    juce::Point<float> globalMaxPoint;

    for (int i = 0; i < scopeSize; ++i)
    {
        if (globalMaxVal < scopeData[i])
        {
            globalMaxBinIdx = i;
            globalMaxVal = scopeData[i];
        }
        const float diff = destScope[i] - scopeDataStorage[i];
        const float coeff = (diff > 0.0f) ? 0.6f : 0.95f;
        scopeDataStorage[i] = destScope[i] - diff * coeff;
    }

    std::vector<juce::Point<float>> points;
    points.reserve(scopeSize / 2 + 1);

    points.emplace_back(leftX,
        juce::jmap(scopeDataStorage[0], mindB, maxdB, bottom, top));

    int i = 0;
    while (i < scopeSize - 1)
    {
        int step = 1 + i / 40;
        step = juce::jlimit(2, 16, step);

        int nextIdx = i + step;
        if (nextIdx >= scopeSize)
            nextIdx = scopeSize - 1;

        float maxVal = scopeDataStorage[i];
        int localMaxIdx = i;

        for (int idx = i + 1; idx <= nextIdx; ++idx)
        {
            if (scopeDataStorage[idx] > maxVal)
            {
                maxVal = scopeDataStorage[idx];
                localMaxIdx = idx;
            }
        }

        const float midIdx = (i + nextIdx) * 0.5f;
        const float f = std::log(midIdx + std::exp(1.0f)) - 1.0f;
        const float x = (f / scopeSizeTransformed) * width + leftX;
        const float y = juce::jmap(maxVal, mindB, maxdB, bottom, top);

        points.emplace_back(x, y);

        if (i < globalMaxBinIdx)
        {
            globalMaxPoint = { x, y };
        }

        i = nextIdx;
    }

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

    if (globalMaxBinIdx > 0)
    {
        const float sampleRate = 48000.0f;

        float refinedBinIdx = static_cast<float>(globalMaxBinIdx);

        if (globalMaxBinIdx > 0 && globalMaxBinIdx < scopeSize - 1)
        {
            float y1 = scopeData[globalMaxBinIdx - 1];
            float y2 = scopeData[globalMaxBinIdx];
            float y3 = scopeData[globalMaxBinIdx + 1];

            float denominator = 2.0f * (2.0f * y2 - y1 - y3);
            if (std::abs(denominator) > 1e-5f)
            {
                float delta = (y3 - y1) / denominator;

                delta = juce::jlimit(-0.5f, 0.5f, delta);

                refinedBinIdx += delta;
            }
        }

        const float freq = (refinedBinIdx * (sampleRate * 0.5f)) / static_cast<float>(scopeSize);

        const float rawPeakDB = scopeData[globalMaxBinIdx];
        const float displayPeakDB = destScope[globalMaxBinIdx];
        const float currentDisplayDB = scopeDataStorage[globalMaxBinIdx];

        juce::String pitchStr = "-";
        if (freq > 20.0f && freq < 20000.0f)
        {
            const float floatNote = 69.0f + 12.0f * std::log2(freq / 440.0f);
            const int midiNote = juce::roundToInt(floatNote);
            const float cents = (floatNote - static_cast<float>(midiNote)) * 100.0f;

            const char* noteNames[] = { "C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B" };
            int noteIndex = midiNote % 12;
            int octave = midiNote / 12 - 1;

            if (noteIndex >= 0 && noteIndex < 12)
            {
                pitchStr = juce::String(noteNames[noteIndex]) + juce::String(octave);
                pitchStr += (cents >= 0 ? " +" : " ") + juce::String(cents, 1) + "c";
            }
        }

        juce::StringArray textLines;
        textLines.add("Freq: " + juce::String(freq, 1) + " Hz (" + pitchStr + ")");
        textLines.add("Raw dB: " + juce::String(rawPeakDB, 2));
        textLines.add("display dB: " + juce::String(displayPeakDB, 2));
        textLines.add("Val: " + juce::String(currentDisplayDB, 2));

        g.setColour(juce::Colours::white.withAlpha(0.8f));
        g.fillEllipse(globalMaxPoint.x - 3.0f, globalMaxPoint.y - 3.0f, 6.0f, 6.0f);

        g.setFont(12.0f);

        float textX = globalMaxPoint.x + 8.0f;
        float textY = globalMaxPoint.y - 10.0f;
        if (textX + 120.0f > bounds.getRight())
            textX = globalMaxPoint.x - 130.0f;

        for (int j = 0; j < textLines.size(); ++j)
        {
            g.drawText(textLines[j],
                static_cast<int>(textX),
                static_cast<int>(textY + j * 14.0f),
                120, 14,
                juce::Justification::centredLeft, false);
        }
    }
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
        scopeDataStorage[i] = mindB;

		scopeData2[i] = mindB;
		scopeDataStorage2[i] = mindB;

        destScope1[i] = mindB;
        destScope2[i] = mindB;
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
    if (!componentHeader.isMouseOver() && !componentHeader.headerFixedButton.getHeaderFixed())
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