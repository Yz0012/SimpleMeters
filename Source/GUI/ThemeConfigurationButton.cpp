
#include "ThemeConfigurationButton.h"

ThemeConfigurationButton::ThemeConfigurationButton()
{
    CreateColoursConfiguration& createColoursConfiguration = CreateColoursConfiguration::getInstance();
}

ThemeConfigurationButton::~ThemeConfigurationButton()
{
}

void ThemeConfigurationButton::paint(juce::Graphics& g)
{
	g.drawImageAt(juce::ImageCache::getFromMemory(BinaryData::themeConfigurationIcon_png, BinaryData::themeConfigurationIcon_pngSize), 0, 0);
}

void ThemeConfigurationButton::mouseDown(const juce::MouseEvent& event)
{
    auto* aw = new juce::AlertWindow(
        "Edit Colour Theme",
        "Modify the hex values (e.g. #RRGGBB) for each colour:",
        juce::AlertWindow::QuestionIcon);

    aw->setOpaque(false);
    aw->setDropShadowEnabled(false);

    for (const auto& colourNode : createColoursConfiguration.currentColourTheme)
    {
        if (colourNode.hasType("Colour"))
        {
            juce::String name = colourNode.getProperty("name", "unnamed");
            juce::String currentHex = colourNode.getProperty("hex", "#000000");

            aw->addTextEditor(name, currentHex, name);
        }
    }

    aw->addButton("OK", 1, juce::KeyPress(juce::KeyPress::returnKey));
    aw->addButton("Cancel", 0, juce::KeyPress(juce::KeyPress::escapeKey));

    aw->enterModalState(true, juce::ModalCallbackFunction::create(
        [this, aw](int result)
        {
            if (result == 1)
            {
                for (auto& colourNode : createColoursConfiguration.currentColourTheme)
                {
                    if (colourNode.hasType("Colour"))
                    {
                        juce::String name = colourNode.getProperty("name", "");
                        if (name.isNotEmpty())
                        {
                            juce::String newHex = aw->getTextEditorContents(name);
                            if (newHex.startsWith("#"))
                                colourNode.setProperty("hex", newHex, nullptr);
                        }
                    }
                }
            }
            delete aw;
        }
    ), true);
}

void ThemeConfigurationButton::mouseEnter(const juce::MouseEvent&)
{
	setMouseCursor(juce::MouseCursor::PointingHandCursor);
}