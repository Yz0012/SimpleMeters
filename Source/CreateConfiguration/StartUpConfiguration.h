
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
			{"StartUpConfig", {{ "name", "StartUpConfig"}},{}},
			{"Position", {{ "name", "Position"}},{}}
		}
	};

	juce::ValueTree currentConfig;

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(StartUpConfiguration);
};
