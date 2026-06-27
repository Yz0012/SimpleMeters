
#pragma once

#include <JuceHeader.h>

class StartUpConfiguration
{
public:
	StartUpConfiguration();
	~StartUpConfiguration();

	juce::File getConfigFile();
	void saveConfig(const juce::ValueTree& tree);
	juce::ValueTree loadConfig();
	juce::ValueTree getCurrentValueTree();

	static StartUpConfiguration& getInstance()
	{
		static StartUpConfiguration instance;
		return instance;
	}

private:

	juce::ValueTree initConfig
	{
		"Config", {{ "name", "Components" }},
		{
			{"StartUpConfig", {
				{ "name", "StartUpConfig"},
				{ "SpectrumAnalyser", "1" }
			}, {} },
			{"Position", {
				{ "name", "Position"},
				{ "SpectrumAnalyserX", "50"},
				{"SpectrumAnalyserY", "50"}
			}, {} },
			{ "ComponentSize",{
				{"name", "ComponentSize"},
				{"SpectrumAnalyserWidth", "500"},
				{"SpectrumAnalyserHeight", "300"},
				{"SpectrumAnalyserMonoWidth", "500"},
				{"SpectrumAnalyserMonoHeight", "300"},
				{"WaveformComponentWidth", "500"},
				{"WaveformComponentHeight", "300"},
				{"VectorOscilloscopeWidth", "300"},
				{"VectorOscilloscopeHeight", "300"},
				{"WaveformChartComponentWidth", "500"},
				{"WaveformChartComponentHeight", "300"},
				{"RMSMeterWidth", "300"},
				{"RMSMeterHeight", "500"},
			}, {} },
			{"MainComponentSize", {
				{ "name", "MainComponentSize"},
				{ "MainComponentWidth", "600" },
				{ "MainComponentHeight", "400" },
			}, {} },
		}
	};

	juce::ValueTree currentConfig;

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(StartUpConfiguration);
};
