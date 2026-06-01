
#pragma once

#include <JuceHeader.h>

#include "../CreateConfiguration/CreateColoursConfiguration.h"

class LookAndFeel : public juce::LookAndFeel_V4
{
public:
    LookAndFeel()
    {
        CreateColoursConfiguration& createColoursConfiguration = CreateColoursConfiguration::getInstance();

        auto popupMenuCat = createColoursConfiguration.currentColourTheme
            .getChildWithProperty("name", "PopupMenu");
        PopupMenuTextColour = juce::Colour(createColoursConfiguration.colourHexToARGBInt(
            popupMenuCat.getChildWithProperty("name", "Text").getProperty("hex").toString(), true));
        PopupMenuBackgroundColour = juce::Colour(createColoursConfiguration.colourHexToARGBInt(
            popupMenuCat.getChildWithProperty("name", "Background").getProperty("hex").toString(), true));

        auto messageWindowCat = createColoursConfiguration.currentColourTheme
            .getChildWithProperty("name", "MessageWindow");
        MessageWindowTextColour = juce::Colour(createColoursConfiguration.colourHexToARGBInt(
            messageWindowCat.getChildWithProperty("name", "Text").getProperty("hex").toString(), true));
        MessageWindowBackgroundColour = juce::Colour(createColoursConfiguration.colourHexToARGBInt(
            messageWindowCat.getChildWithProperty("name", "Background").getProperty("hex").toString(), true));

        auto buttonCat = createColoursConfiguration.currentColourTheme
            .getChildWithProperty("name", "Button");
        ButtonBackgroundColour = juce::Colour(createColoursConfiguration.colourHexToARGBInt(
            buttonCat.getChildWithProperty("name", "Background").getProperty("hex").toString(), true));
        ButtonTextColour = juce::Colour(createColoursConfiguration.colourHexToARGBInt(
            buttonCat.getChildWithProperty("name", "Text").getProperty("hex").toString(), true));
        ButtonPressedBackgroundColour = juce::Colour(createColoursConfiguration.colourHexToARGBInt(
            buttonCat.getChildWithProperty("name", "PressedBackground").getProperty("hex").toString(), true));
        ButtonHoverBackgroundColour = juce::Colour(createColoursConfiguration.colourHexToARGBInt(
            buttonCat.getChildWithProperty("name", "HoverBackground").getProperty("hex").toString(), true));

        auto textEditorCat = createColoursConfiguration.currentColourTheme
            .getChildWithProperty("name", "TextEditor");
        TextEditorBackgroundColour = juce::Colour(createColoursConfiguration.colourHexToARGBInt(
            textEditorCat.getChildWithProperty("name", "Background").getProperty("hex").toString(), true));
        TextEditorTextColour = juce::Colour(createColoursConfiguration.colourHexToARGBInt(
            textEditorCat.getChildWithProperty("name", "Text").getProperty("hex").toString(), true));
        TextEditorHighlightColour = juce::Colour(createColoursConfiguration.colourHexToARGBInt(
            textEditorCat.getChildWithProperty("name", "Highlight").getProperty("hex").toString(), true));
        TextEditorOutlineColour = juce::Colour(createColoursConfiguration.colourHexToARGBInt(
            textEditorCat.getChildWithProperty("name", "Outline").getProperty("hex").toString(), true));
        TextEditorFocusedOutlineColour = juce::Colour(createColoursConfiguration.colourHexToARGBInt(
            textEditorCat.getChildWithProperty("name", "FocusedOutline").getProperty("hex").toString(), true));

        setColour(juce::PopupMenu::backgroundColourId, PopupMenuBackgroundColour);
        setColour(juce::PopupMenu::textColourId, PopupMenuTextColour);

        setColour(juce::AlertWindow::textColourId, MessageWindowTextColour);
        setColour(juce::AlertWindow::outlineColourId, MessageWindowBackgroundColour);
        setColour(juce::AlertWindow::backgroundColourId, MessageWindowBackgroundColour);

        setColour(juce::TextButton::buttonColourId, ButtonBackgroundColour);
        setColour(juce::TextButton::textColourOffId, ButtonTextColour);
        setColour(juce::TextButton::buttonOnColourId, ButtonPressedBackgroundColour);

        setColour(juce::TextEditor::backgroundColourId, TextEditorBackgroundColour);
        setColour(juce::TextEditor::textColourId, TextEditorTextColour);
        setColour(juce::TextEditor::highlightColourId, TextEditorHighlightColour);
        setColour(juce::TextEditor::outlineColourId, TextEditorOutlineColour);
        setColour(juce::TextEditor::focusedOutlineColourId, TextEditorFocusedOutlineColour);

        cascadiaTypeface = juce::Typeface::createSystemTypefaceFor(BinaryData::CascadiaMono_ttf, BinaryData::CascadiaMono_ttfSize);
        setDefaultSansSerifTypeface(cascadiaTypeface);
    }

    void drawPopupMenuBackground(juce::Graphics& g, int width, int height) override
    {
        g.fillAll(findColour(juce::PopupMenu::backgroundColourId));
    }

    void drawAlertBox(juce::Graphics& g, juce::AlertWindow& alert,
        const juce::Rectangle<int>& textArea,
        juce::TextLayout& textLayout) override
    {
        auto bounds = alert.getLocalBounds().reduced(1);
        g.reduceClipRegion(bounds);

        auto iconSpaceUsed = 50;

        g.setColour(alert.findColour(juce::AlertWindow::outlineColourId));
        g.drawRect(alert.getLocalBounds());

        g.setColour(alert.findColour(juce::AlertWindow::backgroundColourId));
        g.fillRect(alert.getLocalBounds());

        g.setColour(alert.findColour(juce::AlertWindow::textColourId));

        juce::Rectangle<int> alertBounds(bounds.getX() + iconSpaceUsed, 30,
            bounds.getWidth(), bounds.getHeight() - getAlertWindowButtonHeight() - 20);

        textLayout.draw(g, alertBounds.toFloat());
    }

    void drawButtonBackground(juce::Graphics& g,
        juce::Button& button,
        const juce::Colour& backgroundColour,
        bool shouldDrawButtonAsHighlighted,
        bool shouldDrawButtonAsDown) override
    {
        juce::Colour targetColour;

        if (shouldDrawButtonAsDown)
            targetColour = ButtonPressedBackgroundColour;
        else if (shouldDrawButtonAsHighlighted)
            targetColour = ButtonHoverBackgroundColour;
        else
            targetColour = ButtonBackgroundColour;

        g.setImageResamplingQuality(juce::Graphics::lowResamplingQuality);
        g.setColour(targetColour);

        auto bounds = button.getLocalBounds().toFloat();
        g.fillRoundedRectangle(bounds, 1.0f);
    }

    void fillTextEditorBackground(juce::Graphics& g, int width, int height, juce::TextEditor& textEditor) override
    {
        g.setColour(textEditor.findColour(juce::TextEditor::backgroundColourId));
        g.fillRect(0, 0, width, height);
    }

private:
    juce::Typeface::Ptr cascadiaTypeface;

    juce::Colour PopupMenuTextColour = juce::Colours::blue;
    juce::Colour PopupMenuBackgroundColour = juce::Colours::black;
    juce::Colour MessageWindowTextColour = juce::Colours::blue;
    juce::Colour MessageWindowBackgroundColour = juce::Colours::black;

    juce::Colour ButtonBackgroundColour = juce::Colours::darkblue;
    juce::Colour ButtonTextColour = juce::Colours::white;
    juce::Colour ButtonPressedBackgroundColour = juce::Colours::green;
    juce::Colour ButtonHoverBackgroundColour = juce::Colours::lightblue;

    juce::Colour TextEditorBackgroundColour = juce::Colours::lightgrey;
    juce::Colour TextEditorTextColour = juce::Colours::black;
    juce::Colour TextEditorHighlightColour = juce::Colours::blueviolet;
    juce::Colour TextEditorOutlineColour = juce::Colours::transparentBlack;
    juce::Colour TextEditorFocusedOutlineColour = juce::Colours::blue;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(LookAndFeel);
};