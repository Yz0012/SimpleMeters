#pragma once
#include <JuceHeader.h>

class WaveformComponentReferenceLine : public juce::Component
{
public:
    WaveformComponentReferenceLine()
    {
        setOpaque(false);
        setInterceptsMouseClicks(false, false);
        setBufferedToImage(true);
    }

    ~WaveformComponentReferenceLine() override = default;

    void setTimeWindow(float newWindowSeconds)
    {
        if (timeWindowSeconds != newWindowSeconds)
        {
            timeWindowSeconds = juce::jmax(0.1f, newWindowSeconds);
            repaint();
        }
    }

    void setGainDb(float newGainDb)
    {
        if (currentGainDb != newGainDb)
        {
            currentGainDb = newGainDb;
            repaint();
        }
    }

    void paint(juce::Graphics& g) override
    {
        const float marginLeft = 45.0f;
        const float marginBottom = 20.0f;

        auto bounds = getLocalBounds().toFloat();
        float waveWidth = bounds.getWidth() - marginLeft;
        float waveHeight = bounds.getHeight() - marginBottom;

        float centerY = waveHeight * 0.5f;
        float halfH = waveHeight * 0.5f;

        g.setFont(14.0f);

        g.setColour(juce::Colour(0xFFFF32D6));
        g.drawLine(marginLeft, 0.0f, marginLeft, waveHeight, 1.0f);

        juce::Array<float> dashLengths{ 8.0f, 8.0f };

        float currentGainLinear = std::pow(10.0f, currentGainDb / 20.0f);

        auto drawDbTick = [&](float tickDb, const juce::String& text, bool isCenter = false)
            {
                float tickLinear = std::pow(10.0f, tickDb / 20.0f);
                float normY = tickLinear * currentGainLinear;

                float yPos = centerY - normY * halfH;
                float yNeg = centerY + normY * halfH;
                float tickLength = 6.0f;

                if (isCenter)
                {
                    g.setColour(juce::Colour(0xFF0091FF));
                    g.drawLine(marginLeft - tickLength, centerY, marginLeft, centerY, 1.0f);
                    g.setColour(juce::Colour(0xFFB7ED88).withAlpha(0.6f));
                    g.drawDashedLine(juce::Line<float>(marginLeft, centerY, bounds.getWidth(), centerY),
                        dashLengths.getRawDataPointer(), dashLengths.size(), 1.0f);
                    g.setColour(juce::Colour(0xFFB7ED88));
                    g.drawText(text, 0, (int)centerY - 10, (int)marginLeft - 8, 20, juce::Justification::centredRight, false);
                }
                else
                {
                    if (yPos < 0.0f || yNeg > waveHeight) return;

                    if (std::abs(yPos - centerY) < 12.0f) return;

                    g.setColour(juce::Colour(0xFF0091FF));
                    g.drawLine(marginLeft - tickLength, yPos, marginLeft, yPos, 1.0f);
                    g.setColour(juce::Colour(0xFF8400FF).withAlpha(0.6f));
                    g.drawDashedLine(juce::Line<float>(marginLeft, yPos, bounds.getWidth(), yPos),
                        dashLengths.getRawDataPointer(), dashLengths.size(), 1.0f);
                    g.setColour(juce::Colour(0xFFB7ED88));
                    g.drawText(text, 0, (int)yPos - 10, (int)marginLeft - 8, 20, juce::Justification::centredRight, false);

                    g.setColour(juce::Colour(0xFF0091FF));
                    g.drawLine(marginLeft - tickLength, yNeg, marginLeft, yNeg, 1.0f);
                    g.setColour(juce::Colour(0xFF8400FF).withAlpha(0.6f));
                    g.drawDashedLine(juce::Line<float>(marginLeft, yNeg, bounds.getWidth(), yNeg),
                        dashLengths.getRawDataPointer(), dashLengths.size(), 1.0f);
                    g.setColour(juce::Colour(0xFFB7ED88));
                    g.drawText(text, 0, (int)yNeg - 10, (int)marginLeft - 8, 20, juce::Justification::centredRight, false);
                }
            };

        if (waveformMode != 8)
        {
            drawDbTick(6.0f, "+6 dB");
            drawDbTick(0.0f, "0 dB");
            drawDbTick(-12.0f, "-12 dB");
            drawDbTick(-24.0f, "-24 dB");
            drawDbTick(-100.0f, "-inf", true);
        }

        g.setColour(juce::Colour(0xFFFF32D6));
        g.drawLine(marginLeft, waveHeight, bounds.getWidth(), waveHeight, 1.0f);

        float rightEdge = bounds.getWidth();

        float timeStep = 1.0f;
        if (timeWindowSeconds <= 2.0f)       timeStep = 0.5f;
        else if (timeWindowSeconds <= 5.0f)  timeStep = 1.0f;
        else if (timeWindowSeconds <= 12.0f) timeStep = 2.0f;
        else if (timeWindowSeconds <= 25.0f) timeStep = 5.0f;
        else                                 timeStep = 10.0f;

        float pxPerSecond = waveWidth / timeWindowSeconds;

        for (float t = 0.0f; t <= timeWindowSeconds + 0.01f; t += timeStep)
        {
            float xPos = rightEdge - (t * pxPerSecond);

            if (xPos >= marginLeft)
            {
                g.setColour(juce::Colour(0xFFFF32D6));
                g.drawLine(xPos, waveHeight, xPos, waveHeight + 8.0f, 1.0f);

                juce::String timeText = (t == 0.0f) ? "0.0 s" : "-" + juce::String(t, (timeStep == 0.5f ? 1 : 0)) + " s";

                g.setColour(juce::Colour(0xFFB7ED88));
                g.drawText(timeText, (int)xPos - 35, (int)waveHeight + 6, 50, 11, juce::Justification::centredTop, false);

                float subStep = timeStep / 4.0f;
                g.setColour(juce::Colour(0xFFB7ED88).withAlpha(0.4f));
                for (int q = 1; q <= 3; ++q)
                {
                    float xSub = xPos - (static_cast<float>(q) * subStep * pxPerSecond);
                    if (xSub >= marginLeft)
                    {
                        g.drawLine(xSub, waveHeight, xSub, waveHeight + 4.0f, 1.0f);
                    }
                }
            }
        }

        g.setColour(juce::Colour(0xFF0091FF));
        g.drawText("- L", bounds.getX(), (int)waveHeight - 6, 50, 11, juce::Justification::centredTop, false);
        g.setColour(juce::Colour(0xFFFF32D6));
        g.drawText("- R", bounds.getX(), (int)waveHeight + 6, 50, 11, juce::Justification::centredTop, false);
    }

    void setWaveformMode(uint16_t modeNum)
    {
        this->waveformMode = modeNum;
    }

private:
    float timeWindowSeconds = 2.0f;
    float currentGainDb = 0.0f;

    uint16_t waveformMode;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(WaveformComponentReferenceLine)
};