#pragma once

#include <JuceHeader.h>

#include "WASAPI/MiniAudioWASAPI.h"
#include "CreateConfiguration/CreateColoursConfiguration.h"
#include "CreateConfiguration/CreateComponentsConfiguration.h"
#include "CreatePushSampleIntoJuceAudioBufferInstance.h"
#include "Components/ComponentManagement.h"
#include "LookAndFeel/LookAndFeel.h"

#include "GUI/Header.h"

class MainComponent : public juce::Component, private juce::ValueTree::Listener
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
	void userTriedToCloseWindow() override;

	void valueTreePropertyChanged(juce::ValueTree& tree, const juce::Identifier& property) override;

	void openSpectrumAnalyser(int x, int y);
	void openWaveformComponent(int x, int y);
	void openVectorOscilloscopeComponent(int x, int y);
	void openWaveformChartComponent(int x, int y);
	void openRMSMeterComponent(int x, int y);

	void stopAndCloseWASAPIDevice();
	void stopWASAPIDevice();

	juce::ComponentDragger dragger;
private:
	juce::ValueTree mainCategory;

	PushSampleIntoJuceAudioBuffer<float>& pushSampleIntoJuceAudioBuffer = CreatePushSampleIntoJuceAudioBufferInstance::getInstance();

	LookAndFeel lookAndFeel;

	Header header;

	juce::TooltipWindow tooltipWindow;

	juce::Colour mainComponentBackgroundColour = juce::Colours::black;

	std::shared_ptr<MiniAudioWASAPI> miniAudioWASAPI;
	std::weak_ptr<MiniAudioWASAPI> weakMiniAudioWASAPI;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainComponent)
};