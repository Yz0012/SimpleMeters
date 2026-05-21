#pragma once

#include "../Components/VectorOscilloscopeComponent/VectorOscilloscopeComponent.h"
#include "../Components/WaveformComponent/WaveformComponent.h"
#include "../Components/Spectrum/SpectrumAnalyser.h"

#include <JuceHeader.h>

class ComponentManagement
{
public:

    static ComponentManagement* getInstance();

    const std::unique_ptr<WaveformComponent>& getWaveformComponent();

    const std::unique_ptr<SpectrumAnalyser>& getSpectrumAnalyser();

    const std::unique_ptr<VectorOscilloscopes>& getVectorOscilloscopes();

    void resetWaveformComponent();

    void resetSpectrumAnalyser();

    void resetVectorOscilloscopes();

    void resetAllComponents();

private:
    ComponentManagement() = default;
    ~ComponentManagement() = default;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ComponentManagement)

    std::unique_ptr<WaveformComponent> waveformComponent;
    std::unique_ptr<SpectrumAnalyser> spectrumAnalyser;
    std::unique_ptr<VectorOscilloscopes> vectorOscilloscopes;
};