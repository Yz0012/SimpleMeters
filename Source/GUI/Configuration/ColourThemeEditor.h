#pragma once
#include <JuceHeader.h>

class ColourThemeEditor : public juce::Component
{
public:
    ColourThemeEditor(juce::ValueTree& categoryNode);
    void applyChanges();
    int getRequiredHeight() const;

private:
    juce::ValueTree& category;

    struct ColourRow : public juce::Component
    {
        juce::Label label;
        juce::TextEditor editor;

        ColourRow(const juce::String& name, const juce::String& hex);
        void resized() override;
    };

    juce::OwnedArray<ColourRow> rows;

    void resized() override;
};