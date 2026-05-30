
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

	static CreateColoursConfiguration& getInstance()
	{
		static CreateColoursConfiguration instance;
		return instance;
	}

	juce::ValueTree initColourTheme
	{
		"ColourTheme", {},
		{
			{ "Colour", {{ "name", "SpectrumAnalyzerBoundaryLine"}, { "hex", "#0091FF" }}},
			{ "Colour", {{ "name", "SpectrumAnalyzerFill"}, { "hex", "#0091FF" }}},

			{ "Colour", {{ "name", "WaveformBoundaryLineL"}, { "hex", "#0091FF" }}},
			{ "Colour", {{ "name", "WaveformGradientColorOfLinesL"}, { "hex", "#FFFFFF" }}},
			{ "Colour", {{ "name", "WaveformFillL"}, { "hex", "#0091FF" }}},
			{ "Colour", {{ "name", "WaveformBoundaryLineR"}, { "hex", "#FF32D6" }}},
			{ "Colour", {{ "name", "WaveformGradientColorOfLinesR"}, { "hex", "#FFFFFF" }}},
			{ "Colour", {{ "name", "WaveformFillR"}, { "hex", "#FF32D6" }}},

			{ "Colour", {{ "name", "WaveformChartBoundaryLineL"}, { "hex", "#0091FF" }}},
			{ "Colour", {{ "name", "WaveformChartGradientColorOfLinesL"}, { "hex", "#FFFFFF" }}},
			{ "Colour", {{ "name", "WaveformChartFillL"}, { "hex", "#0091FF" }}},
			{ "Colour", {{ "name", "WaveformChartBoundaryLineR"}, { "hex", "#FF32D6" }}},
			{ "Colour", {{ "name", "WaveformChartGradientColorOfLinesR"}, { "hex", "#FFFFFF" }}},
			{ "Colour", {{ "name", "WaveformChartFillR"}, { "hex", "#FF32D6" }}},

			{ "Colour", {{ "name", "MainComponentBackground" }, { "hex", "#172027" }}},
			{ "Colour", {{ "name", "Header"}, { "hex", "#0091FF" }}},
			{ "Colour", {{ "name", "Secondary"}, { "hex", "#FF32D6" }}},
			{ "Colour", {{ "name", "Accent1"}, { "hex", "#B7ED88" }}},

			{ "Colour", {{ "name", "PopupMenuBackgroundColour"}, { "hex", "#212A32" }}},
			{ "Colour", {{ "name", "PopupMenuTextColour"}, { "hex", "#6ABD66" }}},

			{ "Colour", {{ "name", "MessageWindowBackgroundColour"}, { "hex", "#212A32" }}},
			{ "Colour", {{ "name", "MessageWindowTextColour"}, { "hex", "#6ABD66" }}},

			{ "Colour", {{ "name", "ButtonBackgroundColour"}, { "hex", "#212A32" }}},
			{ "Colour", {{ "name", "ButtonTextColour"}, { "hex", "#B7ED88" }}},
			{ "Colour", {{ "name", "ButtonPressedBackgroundColour"}, { "hex", "#DEAC47" }}},
			{ "Colour", {{ "name", "ButtonHoverBackgroundColour"}, { "hex", "#DEAC47" }}},

			{ "Colour", {{ "name", "TextEditorBackgroundColour"}, { "hex", "#172027" }}},
			{ "Colour", {{ "name", "TextEditorTextColour"}, { "hex", "#B7ED88" }}},
			{ "Colour", {{ "name", "TextEditorHighlightColour"}, { "hex", "#172027" }}},
			{ "Colour", {{ "name", "TextEditorOutlineColour"}, { "hex", "#172027" }}},
			{ "Colour", {{ "name", "TextEditorFocusedOutlineColour"}, { "hex", "#172027" }}}
		}
	};

	juce::ValueTree currentColourTheme;

private:

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(CreateColoursConfiguration)
};