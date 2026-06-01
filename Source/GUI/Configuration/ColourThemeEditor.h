
#pragma once
#include <JuceHeader.h>

class ColourThemeEditor : public juce::Component {
public:
    ColourThemeEditor(juce::ValueTree& themeToEdit);
    void applyChanges();
    int getRequiredHeight() const;

private:
    juce::ValueTree& theme;
    struct ColourRow {
        juce::Label label;
        juce::TextEditor editor;
    };
    juce::OwnedArray<ColourRow> rows;

    void resized() override;
};