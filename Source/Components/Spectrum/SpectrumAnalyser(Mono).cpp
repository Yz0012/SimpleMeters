
#include "SpectrumAnalyser(Mono).h"

SpectrumAnalyserMono::SpectrumAnalyserMono() : fftLayer(AudioLayerManager<float>::getInstance().getFftDataLayer()), scopeData(fftLayer->getScopeData())
{
    CreateColoursConfiguration& createColoursConfiguration = CreateColoursConfiguration::getInstance();

    spectrumCat = createColoursConfiguration.currentColourTheme
        .getChildWithProperty("name", "SpectrumAnalyzerMono");
    lineColor = juce::Colour(createColoursConfiguration.colourHexToARGBInt(
        spectrumCat.getChildWithProperty("name", "BoundaryLine").getProperty("hex").toString(), false));
    fillColor = juce::Colour(createColoursConfiguration.colourHexToARGBInt(
        spectrumCat.getChildWithProperty("name", "Fill").getProperty("hex").toString(), false));

    for (int i = 0; i < scopeSize; ++i)
    {
        scopeDataStorage[i] = mindB;

        destScope1[i] = mindB;
    }

    lastProcessTime = juce::Time::getMillisecondCounterHiRes();

    addAndMakeVisible(&drawBounds);
    addAndMakeVisible(&componentHeader);
    addAndMakeVisible(&eqReferenceLines);

    spectrumCat.addListener(this);
}

SpectrumAnalyserMono::~SpectrumAnalyserMono()
{
    spectrumCat.removeListener(this);
    fftLayer->setOnFftReadyCallback(nullptr);
}

void SpectrumAnalyserMono::activityCheck()
{
    lastProcessTime = juce::Time::getMillisecondCounterHiRes();

    if (!isTimerRunning())
    {
        startTimerHz(60);
    }
}

void SpectrumAnalyserMono::drawFrame(juce::Graphics& g, juce::Rectangle<int> bounds)
{
    for (int i = 0; i < scopeSize; ++i)
    {
        destScope1[i] = juce::jlimit(mindB, maxdB, scopeData[i] + (2.0f * std::log(1.0f * i + std::exp(1.0f)) - 7) * 3.0f);
    }
    drawSingleCurve(g, bounds, scopeData, scopeDataStorage, destScope1,
        lineColor, fillColor);
}

void SpectrumAnalyserMono::drawSingleCurve(juce::Graphics& g,
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

    globalMaxBinIdx = fftLayer->getAbsoluteMaxPeak().index;
    globalMaxVal = fftLayer->getAbsoluteMaxPeak().value;

    if (globalMaxBinIdx <= 0) return;

    for (int i = 0; i < scopeSize; ++i)
    {
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
        const float freq = fftLayer->extractInterpolationFreq(fftLayer->getAbsoluteMaxPeak().index);

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

void SpectrumAnalyserMono::paint(juce::Graphics& g)
{
    drawFrame(g, drawArea);
}

void SpectrumAnalyserMono::timerCallback()
{
    checkProcessBufferActivity();
    repaint(getLocalBounds());
}

void SpectrumAnalyserMono::callStopTimer()
{
    stopTimer();
}

void SpectrumAnalyserMono::callstartTimerHz(int hz)
{
    startTimerHz(hz);
}

void SpectrumAnalyserMono::scopeDataReset()
{
    for (int i = 0; i < scopeSize; ++i)
    {
        scopeDataStorage[i] = mindB;

        destScope1[i] = mindB;
    }
}

void SpectrumAnalyserMono::checkProcessBufferActivity()
{
    auto now = juce::Time::getMillisecondCounterHiRes();
    if (now - lastProcessTime > 2000.0)
    {
        stopTimer();
    }
}

void SpectrumAnalyserMono::mouseDown(const juce::MouseEvent& event)
{
    if (event.mods.isPopupMenu())
    {
        if (cb == nullptr) return;
        cb();
    }
}

void SpectrumAnalyserMono::mouseEnter(const juce::MouseEvent& event)
{
    drawBounds.setVisible(true);
    componentHeader.setVisible(true);
}

void SpectrumAnalyserMono::mouseExit(const juce::MouseEvent&)
{
    if (!componentHeader.isMouseOver() && !componentHeader.headerFixedButton.getHeaderFixed())
    {
        drawBounds.setVisible(false);
        componentHeader.setVisible(false);
    }
}

void SpectrumAnalyserMono::valueTreePropertyChanged(juce::ValueTree& tree, const juce::Identifier& property)
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