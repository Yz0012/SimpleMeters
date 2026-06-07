#pragma once
#include <JuceHeader.h>

#include "../Source/CircularImageBuffer.h"
#include "../../CreateConfiguration/CreateColoursConfiguration.h"
#include "../../GUI/Components/DrawBounds.h"
#include "../../GUI/Components/ComponentHeader.h"
#include "WaveformComponentReferenceLine.h"

class WaveformComponent : public juce::Component, private juce::ValueTree::Listener
{
public:
    WaveformComponent();
    ~WaveformComponent();

    void paint(juce::Graphics& g) override;
    void mouseDown(const juce::MouseEvent& event) override;
    void mouseEnter(const juce::MouseEvent& event) override;
    void mouseExit(const juce::MouseEvent&) override;

    void valueTreePropertyChanged(juce::ValueTree& tree, const juce::Identifier& property) override;

    void clear();

    void drawWaveform(
        const juce::AudioBuffer<float>& localAudioBuffer,
        const float localAudioBufferRMS);

    void renderNextFrame(juce::Graphics& g, juce::Rectangle<int> bounds);

    std::shared_ptr<CircularImageBuffer> imageRingBuffer;
    std::weak_ptr<CircularImageBuffer> weakBuffer;

    const std::vector<float> dbValues = {
    -6.f, -12.f
    };

    juce::Rectangle<int> tileArea{ 0,0,8,150 };
    juce::Rectangle<int> drawArea{ 0,0,500,150 };

    uint16_t callbackId = 0;

    DrawBounds drawBounds;
	ComponentHeader componentHeader{ juce::String("Waveform") };
	WaveformComponentReferenceLine waveformReferenceLine;

    using Callback = std::function<void()>;
    Callback cb = nullptr;
private:
	juce::ValueTree waveformCat;

    juce::Colour lineColorL = juce::Colours::blueviolet;
    juce::Colour fillColorL = juce::Colours::blueviolet;
    juce::Colour gradientColorOfLinesL = juce::Colours::white;

    juce::Colour lineColorR = juce::Colours::blueviolet;
    juce::Colour fillColorR = juce::Colours::blueviolet;
    juce::Colour gradientColorOfLinesR = juce::Colours::white;

	int tileSize = 0;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(WaveformComponent)
};