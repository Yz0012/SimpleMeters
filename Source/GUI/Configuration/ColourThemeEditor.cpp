
#include "ColourThemeEditor.h"

ColourThemeEditor::ColourThemeEditor(juce::ValueTree& themeToEdit)
    : theme(themeToEdit)
{
    for (const auto& colourNode : theme) {
        if (colourNode.hasType("Colour")) {
            auto row = std::make_unique<ColourRow>();
            juce::String name = colourNode.getProperty("name", "Unnamed");
            juce::String hex = colourNode.getProperty("hex", "#000000");

            row->label.setText(name + ": ", juce::dontSendNotification);
            row->label.attachToComponent(&row->editor, true);
            row->editor.setText(hex);
            row->editor.setColour(juce::TextEditor::backgroundColourId, juce::Colour::fromString("#212A32"));
            row->editor.setColour(juce::TextEditor::textColourId, juce::Colour::fromString("#B7ED88"));

            addAndMakeVisible(row->editor);
            rows.add(std::move(row));
        }
    }
    setSize(500, getRequiredHeight());
}

int ColourThemeEditor::getRequiredHeight() const {
    const int rowHeight = 30;
    const int padding = 10;
    return rows.size() * rowHeight + padding * 2;
}

void ColourThemeEditor::resized() {
    auto area = getLocalBounds().reduced(10);
    const int rowHeight = 30;
    int y = area.getY();
    for (auto* row : rows) {
        auto rowBounds = area.withY(y).withHeight(rowHeight);
        row->editor.setBounds(rowBounds);
        y += rowHeight;
    }
}

void ColourThemeEditor::applyChanges() {
    int index = 0;
    for (auto& colourNode : theme) {
        if (colourNode.hasType("Colour") && index < rows.size()) {
            juce::String newHex = rows[index]->editor.getText();
            if (newHex.startsWith("#") && (newHex.length() == 7 || newHex.length() == 9))
                colourNode.setProperty("hex", newHex, nullptr);
            ++index;
        }
    }
}