
#pragma once

#include <JuceHeader.h>

#include "../CreateConfiguration/CreateColoursConfiguration.h"

class LookAndFeel : public juce::LookAndFeel_V4
{
public:
    LookAndFeel()
    {
        CreateColoursConfiguration& createColoursConfiguration = CreateColoursConfiguration::getInstance();

        PopupMenuTextColour = juce::Colour(createColoursConfiguration.colourHexToARGBInt(
            createColoursConfiguration.currentColourTheme.getChildWithProperty("name", "PopupMenuTextColour").getProperty("hex").toString(), true));
        PopupMenuBackgroundColour = juce::Colour(createColoursConfiguration.colourHexToARGBInt(
            createColoursConfiguration.currentColourTheme.getChildWithProperty("name", "PopupMenuBackgroundColour").getProperty("hex").toString(), true));

        setColour(juce::PopupMenu::backgroundColourId, PopupMenuBackgroundColour);
        setColour(juce::PopupMenu::textColourId, PopupMenuTextColour);
    }

private:
    juce::Colour PopupMenuTextColour = juce::Colours::blue;
    juce::Colour PopupMenuBackgroundColour = juce::Colours::black;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(LookAndFeel);
};