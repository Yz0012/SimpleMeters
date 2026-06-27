
#pragma once

#include <JuceHeader.h>

class CreateColoursConfiguration
{
public:
	CreateColoursConfiguration();
	~CreateColoursConfiguration();

	juce::uint32 colourHexToARGBInt(const juce::String& hexString, bool isRRGGBBAA);
	juce::File getConfigFile();
	void saveConfig(const juce::ValueTree& tree);
	juce::ValueTree loadConfig();
    juce::ValueTree getCurrentValueTree();

	static CreateColoursConfiguration& getInstance()
	{
		static CreateColoursConfiguration instance;
		return instance;
	}

private:

    juce::ValueTree initColourTheme
    {
        "ColourTheme", {},
        {

            { "Category", {{ "name", "SpectrumAnalyzer" }},
                {
                    { "Colour", {{ "name", "BoundaryLine"}, { "hex", "#0091FF" }}},
                    { "Colour", {{ "name", "Fill"},        { "hex", "#0091FF" }}},
                    { "Colour", {{ "name", "BoundaryLineR"}, { "hex", "#FF32D6" }}},
                    { "Colour", {{ "name", "FillR"},        { "hex", "#FF32D6" }}},
                    { "Colour", {{ "name", "BoundaryLineSide"}, { "hex", "#DEAC47" }}},
                    { "Colour", {{ "name", "FillSide"},        { "hex", "#DEAC47" }}},
                }
            },

        { "Category", {{ "name", "SpectrumAnalyzerMono" }},
            {
                { "Colour", {{ "name", "BoundaryLine"}, { "hex", "#0091FF" }}},
                { "Colour", {{ "name", "Fill"},        { "hex", "#0091FF" }}}
            }
        },

        { "Category", {{ "name", "Waveform" }},
            {
                { "Colour", {{ "name", "BoundaryLineL"},          { "hex", "#0091FF" }}},
                { "Colour", {{ "name", "GradientColorOfLinesL"},  { "hex", "#FFFFFF" }}},
                { "Colour", {{ "name", "BoundaryLineR"},          { "hex", "#FF32D6" }}},
                { "Colour", {{ "name", "GradientColorOfLinesR"},  { "hex", "#FFFFFF" }}}
            }
        },

        { "Category", {{ "name", "WaveformChart" }},
            {
                { "Colour", {{ "name", "BoundaryLineL"},          { "hex", "#0091FF" }}},
                { "Colour", {{ "name", "GradientColorOfLinesL"},  { "hex", "#FFFFFF" }}},
                { "Colour", {{ "name", "FillL"},                  { "hex", "#0091FF" }}},
                { "Colour", {{ "name", "BoundaryLineR"},          { "hex", "#FF32D6" }}},
                { "Colour", {{ "name", "GradientColorOfLinesR"},  { "hex", "#FFFFFF" }}},
                { "Colour", {{ "name", "FillR"},                  { "hex", "#FF32D6" }}}
            }
        },

        { "Category", {{ "name", "VectorOscilloscopes" }},
            {
                { "Colour", {{ "name", "Colour"},          { "hex", "#0091FF" }}}
            }
        },

        { "Category", {{ "name", "RMSMeterComponent" }},
            {
                { "Colour", {{ "name", "Bottom"},          { "hex", "#B7ED88" }}},
                { "Colour", {{ "name", "Top"},          { "hex", "#FFFFFF" }}}
            }
        },

        { "Category", {{ "name", "MainComponent" }},
            {
                { "Colour", {{ "name", "Background"}, { "hex", "#172027" }}},
                { "Colour", {{ "name", "Header"},     { "hex", "#0091FF" }}},
                { "Colour", {{ "name", "Secondary"},  { "hex", "#FF32D6" }}},
                { "Colour", {{ "name", "Accent1"},    { "hex", "#B7ED88" }}}
            }
        },

        { "Category", {{ "name", "ComponentBounds" }},
            {
                { "Colour", {{ "name", "ComponentBoundsColour"}, { "hex", "#B7ED88" }}},
            }
        },

        { "Category", {{ "name", "PopupMenu" }},
            {
                { "Colour", {{ "name", "Background"}, { "hex", "#212A32" }}},
                { "Colour", {{ "name", "Text"},       { "hex", "#6ABD66" }}}
            }
        },

        { "Category", {{ "name", "MessageWindow" }},
            {
                { "Colour", {{ "name", "Background"}, { "hex", "#212A32" }}},
                { "Colour", {{ "name", "Text"},       { "hex", "#6ABD66" }}}
            }
        },

        { "Category", {{ "name", "Button" }},
            {
                { "Colour", {{ "name", "Background"},        { "hex", "#212A32" }}},
                { "Colour", {{ "name", "Text"},              { "hex", "#B7ED88" }}},
                { "Colour", {{ "name", "PressedBackground"}, { "hex", "#DEAC47" }}},
                { "Colour", {{ "name", "HoverBackground"},   { "hex", "#DEAC47" }}}
            }
        },

        { "Category", {{ "name", "TextEditor" }},
            {
                { "Colour", {{ "name", "Background"},      { "hex", "#172027" }}},
                { "Colour", {{ "name", "Text"},            { "hex", "#B7ED88" }}},
                { "Colour", {{ "name", "Highlight"},       { "hex", "#172027" }}},
                { "Colour", {{ "name", "Outline"},         { "hex", "#172027" }}},
                { "Colour", {{ "name", "FocusedOutline"},  { "hex", "#172027" }}}
            }
        }
    }
    };

    juce::ValueTree currentColourTheme;

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(CreateColoursConfiguration)
};