#pragma once
#include <JuceHeader.h>

#include "../../GUI/Components/DrawBounds.h"
#include "../../GUI/Components/ComponentHeader.h"
#include "TicksComponent.h"

class RMSMeterComponent : public juce::Component, private juce::Timer
{
public:
    RMSMeterComponent();
    ~RMSMeterComponent() override;

    void paint(juce::Graphics& g) override;
    void resized() override;

    void updateRMSValues(float left, float right, float total);

    DrawBounds drawBounds;
    ComponentHeader componentHeader{ juce::String("RMSMeterComponent") };
	TicksComponent ticksComponent;

    uint16_t callbackId = 0;
private:
    void timerCallback() override;
    float amplitudeToY(float amplitude, float availableHeight) const;

    float rmsValues[3] = { 0.0f, 0.0f, 0.0f };
    float peakValues[3] = { 0.0f, 0.0f, 0.0f };
    juce::uint64 peakTimers[3] = { 0, 0, 0 };

    const float leftMargin = 40.0f;
    const float bottomMargin = 20.0f;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(RMSMeterComponent)
};