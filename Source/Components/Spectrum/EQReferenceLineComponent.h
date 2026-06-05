
#pragma once

#include <JuceHeader.h>

class EQReferenceLineComponent : public juce::Component
{
public:
    EQReferenceLineComponent()
    {
        setOpaque(false);

        sampleRate = 48000.0f;
        scopeSize = 1024;
        scopeSizeTransformed = std::log(static_cast<float>(scopeSize) + std::exp(1.0f)) - 1.0f;
    }

    ~EQReferenceLineComponent() override = default;

    void setMargins(int left, int top, int right, int bottom)
    {
        marginLeft = left;
        marginTop = top;
        marginRight = right;
        marginBottom = bottom;
        repaint();
    }

    juce::Rectangle<int> getPlotArea() const noexcept
    {
        return { marginLeft, marginTop, getWidth() - marginLeft - marginRight, getHeight() - marginTop - marginBottom };
    }

    void paint(juce::Graphics& g) override
    {
        auto plotArea = getPlotArea();
        if (plotArea.getWidth() <= 0 || plotArea.getHeight() <= 0)
            return;

        const float plotWidth = static_cast<float> (plotArea.getWidth());
        const float plotHeight = static_cast<float> (plotArea.getHeight());
        const float leftX = static_cast<float> (plotArea.getX());
        const float topY = static_cast<float> (plotArea.getY());

        juce::Colour majorGridColour(0x20FFFFFF);
        juce::Colour minorGridColour(0x10FFFFFF);
        juce::Colour axisColour = juce::Colour(0xFFFF32D6);
        juce::Colour textColour = juce::Colour(0xFFB7ED88);

        g.setFont(14.0f);

        g.setColour(minorGridColour);
        std::vector<float> minorFreqs = generateMinorFrequencies();
        for (float freq : minorFreqs)
        {
            float x = getXFromFreq(freq, plotArea);
            if (x >= leftX && x <= leftX + plotWidth)
                g.drawVerticalLine(static_cast<int> (x), topY, topY + plotHeight);
        }

        g.setColour(majorGridColour);
        for (float freq : majorFrequencies)
        {
            float x = getXFromFreq(freq, plotArea);
            if (x >= leftX && x <= leftX + plotWidth)
            {
                g.drawVerticalLine(static_cast<int> (x), topY, topY + plotHeight);

                juce::String freqLabel = formatFrequencyLabel(freq);
                juce::Rectangle<float> labelRect(x - 25.0f, topY + plotHeight + 4.0f, 50.0f, 20.0f);
                g.setColour(textColour);
                g.drawFittedText(freqLabel, labelRect.toNearestInt(), juce::Justification::centred, 1);
                g.setColour(majorGridColour);
            }
        }

        g.setColour(minorGridColour);
        for (float db : minorDBs)
        {
            float y = getYFromDB(db, plotArea);
            if (y >= topY && y <= topY + plotHeight)
                g.drawHorizontalLine(static_cast<int> (y), leftX, leftX + plotWidth);
        }

        g.setColour(majorGridColour);
        for (float db : majorDBs)
        {
            float y = getYFromDB(db, plotArea);
            if (y >= topY && y <= topY + plotHeight)
            {
                g.drawHorizontalLine(static_cast<int> (y), leftX, leftX + plotWidth);

                juce::String labelText = juce::String(static_cast<int> (db));
                juce::Rectangle<float> labelRect(0.0f, y - 9.0f, static_cast<float>(marginLeft - 4), 18.0f);
                g.setColour(textColour);
                g.drawFittedText(labelText, labelRect.toNearestInt(), juce::Justification::centredRight, 1);
                g.setColour(majorGridColour);
            }
        }

        g.setColour(axisColour);
        g.drawVerticalLine(static_cast<int> (leftX), topY, topY + plotHeight);
        g.drawHorizontalLine(static_cast<int> (topY + plotHeight), leftX, leftX + plotWidth);

        g.setColour(textColour);
        g.setFont(14.0f);

        juce::Rectangle<int> dBTitileRect(2, plotArea.getY() + plotArea.getHeight() / 2 - 40, 20, 80);
        g.drawFittedText("dB", dBTitileRect, juce::Justification::centred, 1);

        juce::Rectangle<int> freqTitleRect(plotArea.getX() + plotArea.getWidth() / 2 - 60, getHeight() - marginBottom + 15, 120, 20);
        g.drawFittedText("Frequency (Hz)", freqTitleRect, juce::Justification::centred, 1);
    }

    void resized() override
    {
        repaint();
    }

private:

    float sampleRate = 48000.0f;
    int scopeSize = 1024;
    float scopeSizeTransformed;

    const float minFreqHz = 0.0f;
    const float maxFreqHz = 24000.0f;
    const float mindB = -90.0f;
    const float maxdB = 0.0f;

    const std::vector<float> majorFrequencies = { 100.0f, 1000.0f, 10000.0f };
    const std::vector<float> majorDBs = { 0.0f, -45.0f, -90.0f };
    const std::vector<float> minorDBs = { -10.0f, -20.0f, -30.0f, -40.0f,
                                          -50.0f, -60.0f, -70.0f, -80.0f };

    int marginLeft = 50;
    int marginTop = 15;
    int marginRight = 15;
    int marginBottom = 35;

    inline float transformFreq(float freqHz) const noexcept
    {

        float clampedFreq = juce::jlimit(0.0f, maxFreqHz, freqHz);

        float index = clampedFreq * static_cast<float>(scopeSize) / (sampleRate / 2.0f);

        index = juce::jmax(0.0f, index);

        float transformed = std::log(index + std::exp(1.0f)) - 1.0f;

        return transformed / scopeSizeTransformed;
    }

    inline float getXFromFreq(float freqHz, const juce::Rectangle<int>& plotArea) const noexcept
    {
        float t = transformFreq(freqHz);
        float plotWidth = static_cast<float> (plotArea.getWidth());
        return static_cast<float> (plotArea.getX()) + t * plotWidth;
    }

    inline float getYFromDB(float dB, const juce::Rectangle<int>& plotArea) const noexcept
    {
        const float normalized = (dB - mindB) / (maxdB - mindB);
        const float plotHeight = static_cast<float> (plotArea.getHeight());
        return static_cast<float> (plotArea.getY()) + plotHeight * (1.0f - normalized);
    }

    juce::String formatFrequencyLabel(float freqHz) const
    {
        if (freqHz >= 1000.0f)
        {
            if (std::abs(freqHz - 1000.0f) < 0.1f)  return "1k";
            if (std::abs(freqHz - 10000.0f) < 0.1f) return "10k";
            return juce::String(static_cast<int> (freqHz / 1000.0f)) + "k";
        }
        return juce::String(static_cast<int> (freqHz));
    }

    std::vector<float> generateMinorFrequencies() const
    {
        std::vector<float> freqs;

        const float start1 = 0.0f;
        const float end1 = 100.0f;
        const float step1 = (end1 - start1) / 6.0f;
        for (int i = 1; i <= 5; ++i)
            freqs.push_back(start1 + i * step1);

        const float start2 = 100.0f;
        const float end2 = 1000.0f;
        const float step2 = (end2 - start2) / 6.0f;
        for (int i = 1; i <= 5; ++i)
            freqs.push_back(start2 + i * step2);

        const float start3 = 1000.0f;
        const float end3 = 10000.0f;
        const float step3 = (end3 - start3) / 6.0f;
        for (int i = 1; i <= 5; ++i)
            freqs.push_back(start3 + i * step3);

        const float start4 = 10000.0f;
        const float end4 = 20000.0f;
        const float step4 = (end4 - start4) / 6.0f;
        for (int i = 1; i <= 5; ++i)
            freqs.push_back(start4 + i * step4);

        freqs.push_back(20000.0f);
        freqs.push_back(24000.0f);

        return freqs;
    }

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(EQReferenceLineComponent)
};