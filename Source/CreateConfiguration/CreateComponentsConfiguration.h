
#pragma once

#include <JuceHeader.h>

class CreateComponentsConfiguration
{
public:
	CreateComponentsConfiguration();
	~CreateComponentsConfiguration();

	juce::File getConfigFile();
	void saveConfig(const juce::ValueTree& tree);
	juce::ValueTree loadConfig();

	static CreateComponentsConfiguration& getInstance()
	{
		static CreateComponentsConfiguration instance;
		return instance;
	}

	juce::ValueTree initComponentSettings
	{
		"ComponentSettings", {},
		{
			{ "Category", {{ "name", "SpectrumAnalyzer" }, { "value", 0 }}},
			{ "Category", {{ "name", "Waveform" },         { "value", 0 }}},
			{ "Category", {{ "name", "WaveformChart" },    { "value", 0 }}},
			{ "Category", {{ "name", "VectorOscilloscopes" }, { "value", 0 }}}
		}
	};

	juce::ValueTree currentComponentSettings;
private:
	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(CreateComponentsConfiguration);
};