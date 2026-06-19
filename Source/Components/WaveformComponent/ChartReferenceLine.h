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

        const float dbValues[] = { 0.0f, -3.0f };

        for (float db : dbValues)
        {
            float amplitude = std::pow(10.0f, db / 20.0f);
            float yTop = centerY - (amplitude * halfHeight);
            float yBottom = centerY + (amplitude * halfHeight);
            if (db == 0.0f)
            {
                g.setColour(juce::Colour(0xFFFF32D6));
                g.drawLine(plotArea.getX(), yBottom, plotArea.getRight(), yBottom);
            }
            else
            {
                juce::String dbStr = juce::String(db, 0) + " dB";
                g.setColour(juce::Colour(0xFFB7ED88));
                g.drawText(dbStr, bounds.getX(), yTop - 6.0f, marginLeft - 5.0f, 12.0f, juce::Justification::centredRight, false);
                g.drawText(dbStr, bounds.getX(), yBottom - 6.0f, marginLeft - 5.0f, 12.0f, juce::Justification::centredRight, false);

                g.setColour(juce::Colour(0xFF8400FF).withAlpha(0.6f));
                g.drawDashedLine(juce::Line<float>(plotArea.getX(), yTop, plotArea.getRight(), yTop), dashLengths, 2, 1.0f);
                g.drawDashedLine(juce::Line<float>(plotArea.getX(), yBottom, plotArea.getRight(), yBottom), dashLengths, 2, 1.0f);
            }
        }

        const float totalSamples = 1000.0f;
        const float sampleMarks[] = { 250.0f, 500.0f, 750.0f };

        for (float s : sampleMarks)
        {
            float ratio = s / totalSamples;
            float xPos = plotArea.getX() + ratio * plotArea.getWidth();

            g.setColour(juce::Colour(0xFFB7ED88).withAlpha(0.6f));
            g.drawDashedLine(juce::Line<float>(xPos, plotArea.getY(), xPos, plotArea.getBottom()), dashLengths, 2, 1.0f);

            g.setColour(juce::Colour(0xFFB7ED88));
            g.drawText(juce::String(static_cast<int>(s)),
                xPos - 20.0f, plotArea.getBottom() + 2.0f, 40.0f, marginBottom,
                juce::Justification::centredTop, false);
        }

        g.setColour(juce::Colour(0xFFFF32D6));
        g.drawLine(plotArea.getX(), plotArea.getY(), plotArea.getX(), plotArea.getBottom());

        g.setColour(juce::Colour(0xFF0091FF));
        g.drawText("- L", bounds.getX(), (int)plotArea.getBottom() - 6, 50, 11, juce::Justification::centredTop, false);
        g.setColour(juce::Colour(0xFFFF32D6));
        g.drawText("- R", bounds.getX(), (int)plotArea.getBottom() + 6, 50, 11, juce::Justification::centredTop, false);
    }
};