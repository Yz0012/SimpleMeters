#pragma once

#include <JuceHeader.h>

class OscilloscopeReferenceLines : public juce::Component
{
public:
    OscilloscopeReferenceLines()
    {
        setOpaque(false);
        setInterceptsMouseClicks(false, false);
    }

    void paint(juce::Graphics& g) override
    {
        const int w = getWidth();
        const int h = getHeight();
        if (w <= 0 || h <= 0) return;

        const float cx = w * 0.5f;
        const float cy = h * 0.5f;
        const float rx = w * 0.5f;
        const float ry = h * 0.5f;

        juce::Path diamond;
        diamond.startNewSubPath(cx + rx, cy);
        diamond.lineTo(cx, cy + ry);
        diamond.lineTo(cx - rx, cy);
        diamond.lineTo(cx, cy - ry);
        diamond.closeSubPath();

        g.setColour(lineColor.withAlpha(0.3f));
        g.strokePath(diamond, juce::PathStrokeType(1.2f));

        auto dbToAmp = [](float db) { return std::pow(10.0f, db / 20.0f); };
        const float amp3 = dbToAmp(-3.0f);
        const float amp6 = dbToAmp(-6.0f);
        const float amp9 = dbToAmp(-9.0f);

        g.setColour(lineColor3.withAlpha(0.5f));
        g.drawEllipse(cx - rx * amp3, cy - ry * amp3,
            rx * amp3 * 2.0f, ry * amp3 * 2.0f, 1.2f);

        g.setColour(lineColor2.withAlpha(0.5f));
        g.drawEllipse(cx - rx * amp6, cy - ry * amp6,
            rx * amp6 * 2.0f, ry * amp6 * 2.0f, 1.2f);

        g.setColour(lineColor.withAlpha(0.5f));
        g.drawEllipse(cx - rx * amp9, cy - ry * amp9,
            rx * amp9 * 2.0f, ry * amp9 * 2.0f, 1.2f);

        g.setColour(lineColor.withAlpha(0.8f));
        g.drawLine(cx, 0.0f, cx, static_cast<float>(h), 0.8f);
        g.drawLine(0.0f, cy, static_cast<float>(w), cy, 0.8f);

        const float endX = w - 40.0f;
        float y = 10.0f;
        const float step = 10.0f;

        g.setColour(lineColor3);
        g.setFont(14.0f);
        g.drawText("-3 dB", static_cast<int>(endX - 15), static_cast<int>(y - 10), 50, 18, juce::Justification::centredLeft);

        y += step;
        g.setColour(lineColor2);
        g.setColour(lineColor2);
        g.drawText("-6 dB", static_cast<int>(endX - 15), static_cast<int>(y - 10), 50, 18, juce::Justification::centredLeft);

        y += step;
        g.setColour(lineColor);
        g.setColour(lineColor);
        g.drawText("-9 dB", static_cast<int>(endX - 15), static_cast<int>(y - 10), 50, 18, juce::Justification::centredLeft);
    }

private:
    juce::Colour lineColor = juce::Colour(0xFF0091FF);
    juce::Colour lineColor2 = juce::Colour(0xFFB7ED88);
    juce::Colour lineColor3 = juce::Colour(0xFFFF32D6);
};