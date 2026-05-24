#pragma once

#include "../Components/VectorOscilloscopeComponent/VectorOscilloscopeComponent.h"
#include "../Components/WaveformComponent/WaveformComponent.h"
#include "../Components/Spectrum/SpectrumAnalyser.h"
#include "../Components/WaveformComponent/WaveformChartComponent.h"

#include <JuceHeader.h>

class ComponentManagement
{
public:

    static ComponentManagement& getInstance();

    const std::unique_ptr<WaveformComponent>& getWaveformComponent();

    const std::unique_ptr<SpectrumAnalyser>& getSpectrumAnalyser();

    const std::unique_ptr<VectorOscilloscopes>& getVectorOscilloscopes();

    const std::unique_ptr<WaveformChartComponent>& getWaveformChartComponent();

    void resetWaveformComponent();

    void resetSpectrumAnalyser();

    void resetVectorOscilloscopes();

    void resetWaveformChartComponent();

    void resetAllComponents();

private:
    ComponentManagement() = default;
    ~ComponentManagement() = default;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ComponentManagement)

    std::unique_ptr<WaveformComponent> waveformComponent;
    std::unique_ptr<SpectrumAnalyser> spectrumAnalyser;
    std::unique_ptr<VectorOscilloscopes> vectorOscilloscopes;
    std::unique_ptr<WaveformChartComponent> waveformChartComponent;
};