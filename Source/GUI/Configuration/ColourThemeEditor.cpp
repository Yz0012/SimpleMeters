#include "ColourThemeEditor.h"

ColourThemeEditor::ColourRow::ColourRow(const juce::String& name, const juce::String& hex)
{
    label.setText(name + ":", juce::dontSendNotification);
    label.setJustificationType(juce::Justification::centredRight);
    label.setColour(juce::Label::textColourId, juce::Colours::white);
    editor.setText(hex);
    editor.setColour(juce::TextEditor::outlineColourId, juce::Colour(0xFF172027));
    editor.setColour(juce::TextEditor::focusedOutlineColourId, juce::Colour(0xFF172027));

    addAndMakeVisible(label);
    addAndMakeVisible(editor);
}

void ColourThemeEditor::ColourRow::resized()
{
    auto area = getLocalBounds();
    auto labelArea = area.removeFromLeft(180);
    label.setBounds(labelArea);
    editor.setBounds(area);
}

ColourThemeEditor::ColourThemeEditor(juce::ValueTree& categoryNode)
    : category(categoryNode)
{
    for (const auto& colourNode : category)
    {
        if (colourNode.hasType("Colour"))
        {
            juce::String name = colourNode.getProperty("name", "Unnamed");
            juce::String hex = colourNode.getProperty("hex", "#000000");
            auto row = std::make_unique<ColourRow>(name, hex);
            addAndMakeVisible(row.get());
            rows.add(std::move(row));
        }
    }
    setSize(500, getRequiredHeight());
}

ColourThemeEditor::~ColourThemeEditor()
{
    DBG("ColourThemeEditor shutdown");
}

int ColourThemeEditor::getRequiredHeight() const
{
    const int rowHeight = 32;
    const int padding = 10;
    return rows.size() * rowHeight + padding * 2;
}

void ColourThemeEditor::resized()
{
    auto area = getLocalBounds().reduced(10);
    const int rowHeight = 32;
    int y = area.getY();
    for (auto* row : rows)
    {
        row->setBounds(area.getX(), y, area.getWidth(), rowHeight);
        y += rowHeight;
    }
}

void ColourThemeEditor::applyChanges()
{
    int index = 0;
    for (auto& colourNode : category)
    {
        if (colourNode.hasType("Colour") && index < rows.size())
        {
            juce::String newHex = rows[index]->editor.getText();
            if (newHex.startsWith("#") && (newHex.length() == 7 || newHex.length() == 9))
                colourNode.setProperty("hex", newHex, nullptr);
            ++index;
        }
    }
}