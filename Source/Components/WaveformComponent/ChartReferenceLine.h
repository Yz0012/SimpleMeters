#pragma once
#include <JuceHeader.h>

class ChartReferenceLine : public juce::Component
{
public:
    ChartReferenceLine()
    {
        setOpaque(false);
        setInterceptsMouseClicks(false, false);
        setBufferedToImage(true);
    }

    ~ChartReferenceLine() override = default;

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

        juce::Rectangle<float> bounds = getLocalBounds().toFloat();

        juce::Rectangle<float> plotArea = bounds.withTrimmedLeft(marginLeft)
            .withTrimmedBottom(marginBottom);

        const float centerY = plotArea.getY() + plotArea.getHeight() * 0.5f;
        const float halfHeight = plotArea.getHeight() * 0.5f;

        g.setFont(14.0f);

        const float dashLengths[2] = { 8.0f, 8.0f };

        g.setColour(juce::Colour(0xFFB7ED88).withAlpha(0.6f));
        g.drawDashedLine(juce::Line<float>(plotArea.getX(), centerY, plotArea.getRight(), centerY), dashLengths, 2, 1.0f);

        g.setColour(juce::Colour(0xFFB7ED88));
        g.drawText("-inf", bounds.getX(), centerY - 6.0f, marginLeft - 5.0f, 12.0f, juce::Justification::centredRight, false);

        auto drawDbTick = [&](float tickDb, const juce::String& text, bool isCenter = false)
            {
                float amplitude = std::pow(10.0f, (tickDb + currentGainDb) / 20.0f);
                float yTop = centerY - amplitude * halfHeight;
                float yBottom = centerY + amplitude * halfHeight;

                if (yTop < 0.0f || yBottom > plotArea.getBottom()) return;
                if (std::abs(yTop - centerY) < 12.0f) return;

                juce::String dbStr = juce::String(tickDb, 0) + " dB";
                g.setColour(juce::Colour(0xFFB7ED88));
                g.drawText(dbStr, bounds.getX(), yTop - 6.0f, marginLeft - 5.0f, 12.0f,
                    juce::Justification::centredRight, false);
                g.drawText(dbStr, bounds.getX(), yBottom - 6.0f, marginLeft - 5.0f, 12.0f,
                    juce::Justification::centredRight, false);

                g.setColour(juce::Colour(0xFF8400FF).withAlpha(0.6f));
                g.drawDashedLine(juce::Line<float>(plotArea.getX(), yTop, plotArea.getRight(), yTop),
                    dashLengths, 2, 1.0f);
                g.drawDashedLine(juce::Line<float>(plotArea.getX(), yBottom, plotArea.getRight(), yBottom),
                    dashLengths, 2, 1.0f);
            };

        drawDbTick(6.0f, "+6 dB");
        drawDbTick(-3.0f, "-3 dB");
        drawDbTick(-12.0f, "-12 dB");
        drawDbTick(-24.0f, "-24 dB");
        drawDbTick(-48.0f, "-48 db");

        g.setColour(juce::Colour(0xFFFF32D6));
        g.drawLine(plotArea.getX(), plotArea.getY(), plotArea.getX(), plotArea.getBottom());

        g.setColour(juce::Colour(0xFF0091FF));
        g.drawText("- L", bounds.getX(), (int)plotArea.getBottom() - 6, 50, 11, juce::Justification::centredTop, false);
        g.setColour(juce::Colour(0xFFFF32D6));
        g.drawText("- R", bounds.getX(), (int)plotArea.getBottom() + 6, 50, 11, juce::Justification::centredTop, false);
    }

private:
    float currentGainDb = 0.0f;
};