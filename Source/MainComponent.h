#pragma once

#include <JuceHeader.h>

#include "WASAPI/MiniAudioWASAPI.h"
#include "CreateConfiguration/CreateColoursConfiguration.h"
#include "LookAndFeel/LookAndFeel.h"

#include "GUI/Header.h"

class MainComponent : public juce::Component
{
public:
    MainComponent();
    ~MainComponent() override;

    void paint (juce::Graphics&) override;
    void resized() override;

	void mouseDown(const juce::MouseEvent& event) override;
	void mouseDrag(const juce::MouseEvent& event) override;
	void mouseEnter(const juce::MouseEvent&) override;
	void mouseExit(const juce::MouseEvent&) override;

	void openSpectrumAnalyser(int x, int y);
	void openWaveformComponent(int x, int y);

	void userTriedToCloseWindow() override;

	juce::ComponentDragger dragger;
private:
	std::unique_ptr<LookAndFeel> lookAndFeel;
	std::unique_ptr<Header> header;

	juce::TooltipWindow tooltipWindow;

	CreateColoursConfiguration* createColoursConfiguration;
	juce::Colour mainComponentBackgroundColour = juce::Colours::black;

	std::shared_ptr<MiniAudioWASAPI> miniAudioWASAPI;
	std::weak_ptr<MiniAudioWASAPI> weakMiniAudioWASAPI;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainComponent)
};