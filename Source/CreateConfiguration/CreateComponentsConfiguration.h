
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
            { "Category", { { "name", "SpectrumAnalyzer" } },
                {
                    { "Component", { { "type", "Main"            }, { "initX", 0 }, { "initY", 0 }, { "width", 500 }, { "height", 150 } } },
                    { "Component", { { "type", "drawBounds"      }, { "initX", 0 }, { "initY", 0 }, { "width", 500 }, { "height", 150 } } },
                    { "Component", { { "type", "drawArea"        }, { "initX", 0 }, { "initY", 50 }, { "width", 500 }, { "height", 100 } } },
                    { "Component", { { "type", "componentHeader" }, { "initX", 0 }, { "initY", 0 }, { "width", 500 }, { "height", 50 } } }
                }
            },

        { "Category", { { "name", "Waveform" } },
            {
                { "Component", { { "type", "Main"            }, { "initX", 0 }, { "initY", 0 }, { "width", 500 }, { "height", 150 } } },
                { "Component", { { "type", "drawBounds"      }, { "initX", 0 }, { "initY", 0 }, { "width", 500 }, { "height", 150 } } },
                { "Component", { { "type", "tileArea"        }, { "initX", 0 }, { "initY", 0 }, { "width", 16  }, { "height", 100 } } },
                { "Component", { { "type", "drawArea"        }, { "initX", 0 }, { "initY", 50 }, { "width", 500 }, { "height", 100 } } },
                { "Component", { { "type", "componentHeader" }, { "initX", 0 }, { "initY", 0 }, { "width", 500 }, { "height", 50 } } }
            }
        },

        { "Category", { { "name", "WaveformChart" } },
            {
                { "Component", { { "type", "Main"            }, { "initX", 0 }, { "initY", 0 }, { "width", 500 }, { "height", 150 } } },
                { "Component", { { "type", "drawBounds"      }, { "initX", 0 }, { "initY", 0 }, { "width", 500 }, { "height", 150 } } },
                { "Component", { { "type", "drawArea"        }, { "initX", 0 }, { "initY", 50 }, { "width", 500 }, { "height", 100 } } },
                { "Component", { { "type", "componentHeader" }, { "initX", 0 }, { "initY", 0 }, { "width", 500 }, { "height", 50 } } }
            }
        },

        { "Category", { { "name", "VectorOscilloscopes" } },
            {
                { "Component", { { "type", "Main"            }, { "initX", 0 }, { "initY", 0 }, { "width", 300 }, { "height", 300 } } },
                { "Component", { { "type", "drawBounds"      }, { "initX", 0 }, { "initY", 0 }, { "width", 300 }, { "height", 300 } } },
                { "Component", { { "type", "componentHeader" }, { "initX", 0 }, { "initY", 0 }, { "width", 300 }, { "height", 50 } } }
            }
        }
    }
    };

	juce::ValueTree currentComponentSettings;
private:
	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(CreateComponentsConfiguration);
};