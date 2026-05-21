
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

	static CreateColoursConfiguration *getInstance()
	{
		static CreateColoursConfiguration instance;
		return &instance;
	}

	juce::ValueTree initColourTheme
	{
		"ColourTheme", {},
		{
			{ "Colour", {{ "name", "SpectrumAnalyzerBoundaryLine"}, { "hex", "#8400FF" }}},
			{ "Colour", {{ "name", "SpectrumAnalyzerFill"}, { "hex", "#8400FF" }}},
			{ "Colour", {{ "name", "WaveformBoundaryLine"}, { "hex", "#0091FF" }}},
			{ "Colour", {{ "name", "WaveformGradientColorOfLines"}, { "hex", "#FFFFFF" }}},
			{ "Colour", {{ "name", "WaveformFill"}, { "hex", "#0091FF" }}},
			{ "Colour", {{ "name", "MainComponentBackground" }, { "hex", "#172027" }}},
			{ "Colour", {{ "name", "Header"}, { "hex", "#0091FF" }}},
			{ "Colour", {{ "name", "Secondary"}, { "hex", "#FF32D6" }}},
			{ "Colour", {{ "name", "Accent1"}, { "hex", "#B7ED88" }}},
			{ "Colour", {{ "name", "PopupMenuBackgroundColour"}, { "hex", "#212A32" }}},
			{ "Colour", {{ "name", "PopupMenuTextColour"}, { "hex", "#6ABD66" }}}
		}
	};

	juce::ValueTree currentColourTheme;

private:

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(CreateColoursConfiguration)
};