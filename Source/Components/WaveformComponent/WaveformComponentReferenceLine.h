#pragma once
#include <JuceHeader.h>

class WaveformComponentReferenceLine : public juce::Component
{
public:
    WaveformComponentReferenceLine()
    {
        setOpaque(false);
        setInterceptsMouseClicks(false, false);
    }

    ~WaveformComponentReferenceLine() override = default;

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

        float gain0dB = 1.0f;
        float gainMinus3dB = std::pow(10.0f, -3.0f / 20.0f);

        juce::Array<float> dashLengths;
        dashLengths.add(8.0f);
        dashLengths.add(8.0f);

        auto drawDbTick = [&](float gain, const juce::String& text, bool isCenter = false)
            {
                float yPos = centerY - gain * halfH;
                float yNeg = centerY + gain * halfH;
                float tickLength = 6.0f;

                g.setColour(juce::Colour(0xFF0091FF));

                if (isCenter)
                {
                    g.drawLine(marginLeft - tickLength, centerY, marginLeft, centerY, 1.0f);

                    g.setColour(juce::Colour(0xFFB7ED88).withAlpha(0.6f));
                    g.drawDashedLine(juce::Line<float>(marginLeft, centerY, bounds.getWidth(), centerY),
                        dashLengths.getRawDataPointer(), dashLengths.size(), 1.0f);

                    g.setColour(juce::Colour(0xFFB7ED88));
                    g.drawText(text, 0, (int)centerY - 10, (int)marginLeft - 8, 20, juce::Justification::centredRight, false);
                }
                else
                {
                    g.drawLine(marginLeft - tickLength, yPos, marginLeft, yPos, 1.0f);
                    g.drawLine(marginLeft - tickLength, yNeg, marginLeft, yNeg, 1.0f);

                    g.setColour(juce::Colour(0xFF8400FF).withAlpha(0.6f));
                    g.drawDashedLine(juce::Line<float>(marginLeft, yPos, bounds.getWidth(), yPos),
                        dashLengths.getRawDataPointer(), dashLengths.size(), 1.0f);
                    g.drawDashedLine(juce::Line<float>(marginLeft, yNeg, bounds.getWidth(), yNeg),
                        dashLengths.getRawDataPointer(), dashLengths.size(), 1.0f);

                    g.setColour(juce::Colour(0xFFB7ED88));
                    g.drawText(text, 0, (int)yPos - 10, (int)marginLeft - 8, 20, juce::Justification::centredRight, false);
                    g.drawText(text, 0, (int)yNeg - 10, (int)marginLeft - 8, 20, juce::Justification::centredRight, false);
                }
            };

        drawDbTick(gainMinus3dB, "-3 dB");
        drawDbTick(0.0f, "-inf", true);

        const float pxPerSecond = 768.0f;
        const float pxPerQuarter = 192.0f;

        g.setColour(juce::Colour(0xFFFF32D6));
        g.drawLine(marginLeft, waveHeight, bounds.getWidth(), waveHeight, 1.0f);

        float rightEdge = bounds.getWidth();

        int maxSeconds = static_cast<int>(waveWidth / pxPerSecond);

        for (int s = 0; s <= maxSeconds; ++s)
        {
            float xSecond = rightEdge - (static_cast<float>(s) * pxPerSecond);

            if (xSecond >= marginLeft)
            {
                g.setColour(juce::Colour(0xFFFF32D6));
                g.drawLine(xSecond, waveHeight, xSecond, waveHeight + 8.0f, 1.0f);

                juce::String timeText = (s == 0) ? "0.0 s" : "-" + juce::String(static_cast<float>(s), 1) + " s";

                g.setColour(juce::Colour(0xFFB7ED88));
                g.drawText(timeText, (int)xSecond - 35, (int)waveHeight + 6, 50, 11, juce::Justification::centredTop, false);

                g.setColour(juce::Colour(0xFFB7ED88));
                for (int q = 1; q <= 3; ++q)
                {
                    float xQuarter = xSecond - (static_cast<float>(q) * pxPerQuarter);
                    if (xQuarter >= marginLeft)
                    {
                        g.drawLine(xQuarter, waveHeight, xQuarter, waveHeight + 4.0f, 1.0f);
                    }
                }
            }
        }

        g.setColour(juce::Colour(0xFF0091FF));
        g.drawText("- L", bounds.getX(), (int)waveHeight - 6, 50, 11, juce::Justification::centredTop, false);
        g.setColour(juce::Colour(0xFFFF32D6));
        g.drawText("- R", bounds.getX(), (int)waveHeight + 6, 50, 11, juce::Justification::centredTop, false);
    }

private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(WaveformComponentReferenceLine)
};