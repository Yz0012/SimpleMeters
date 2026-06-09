#pragma once
#include <JuceHeader.h>

class TicksComponent : public juce::Component
{
public:
    TicksComponent()
    {
        setInterceptsMouseClicks(false, false);
    }

    ~TicksComponent() override = default;

    void paint(juce::Graphics& g) override
    {
        auto bounds = getLocalBounds().toFloat();

        const float leftMargin = 40.0f;
        const float bottomMargin = 20.0f;

        float meterWidth = bounds.getWidth() - leftMargin;
        float meterHeight = bounds.getHeight() - bottomMargin;

        g.setColour(juce::Colour(0xFFB7ED88));

        g.setFont(14.0f);
        const std::vector<float> dbTicks = { 6.0f, 0.0f, -6.0f, -12.0f, -24.0f, -48.0f };

        for (float db : dbTicks)
        {
            float yPos = juce::jmap(db, -60.0f, 12.0f, meterHeight, 0.0f);
            g.drawHorizontalLine(static_cast<int>(yPos), leftMargin - 8.0f, leftMargin);

            juce::String text = (db > 0.0f) ? ("+" + juce::String(db, 0)) : juce::String(db, 0);
            g.drawText(text, 0, static_cast<int>(yPos) - 6, leftMargin - 12.0f, 12, juce::Justification::centredRight);
        }

        g.setFont(14.0f);
        juce::String labels[3] = { "L", "R", "Total" };

        float barWidth = meterWidth * 0.22f;
        float gap = (meterWidth - 3.0f * barWidth) / 4.0f;

        for (int i = 0; i < 3; ++i)
        {
            float xOffset = leftMargin + gap + i * (barWidth + gap);
            g.drawText(labels[i], xOffset, meterHeight, barWidth, bottomMargin, juce::Justification::centred);
        }
    }

    void resized() override {}

private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(TicksComponent)
};