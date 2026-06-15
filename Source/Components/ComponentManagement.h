#pragma once

#include <JuceHeader.h>

#include "../Components/VectorOscilloscopeComponent/VectorOscilloscopeComponent.h"
#include "../Components/WaveformComponent/WaveformComponent.h"
#include "../Components/Spectrum/SpectrumAnalyser.h"
#include "../Components/WaveformComponent/WaveformChartComponent.h"
#include "../Components/LoudnessMeterComponents/RMSMeterComponent.h"
#include "../Components/Spectrum/SpectrumAnalyser(Mono).h"

class ComponentManagement
{
public:
    static ComponentManagement& getInstance();

    std::shared_ptr<WaveformComponent> getWaveformComponent();
    std::shared_ptr<SpectrumAnalyser> getSpectrumAnalyser();
    std::shared_ptr<VectorOscilloscopes> getVectorOscilloscopes();
    std::shared_ptr<WaveformChartComponent> getWaveformChartComponent();
	std::shared_ptr<RMSMeterComponent> getRMSMeterComponent();
    std::shared_ptr<SpectrumAnalyserMono> getSpectrumAnalyserMono();

    void resetWaveformComponent();
    void resetSpectrumAnalyser();
    void resetVectorOscilloscopes();
    void resetWaveformChartComponent();
	void resetRMSMeterComponent();
    void resetSpectrumAnalyserMono();
    void resetAllComponents();

private:
    ComponentManagement() = default;
    ~ComponentManagement() = default;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ComponentManagement)

    std::shared_ptr<WaveformComponent> waveformComponent;
    std::shared_ptr<SpectrumAnalyser> spectrumAnalyser;
    std::shared_ptr<VectorOscilloscopes> vectorOscilloscopes;
    std::shared_ptr<WaveformChartComponent> waveformChartComponent;
	std::shared_ptr<RMSMeterComponent> rmsMeterComponent;
    std::shared_ptr<SpectrumAnalyserMono> spectrumAnalyserMono;
};