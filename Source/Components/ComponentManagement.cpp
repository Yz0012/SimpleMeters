#include "ComponentManagement.h"

ComponentManagement& ComponentManagement::getInstance()
{
    static ComponentManagement instance;
    return instance;
}

const std::unique_ptr<WaveformComponent>& ComponentManagement::getWaveformComponent()
{
    if (waveformComponent == nullptr)
    {
        waveformComponent = std::make_unique<WaveformComponent>();
    }
    return waveformComponent;
}

const std::unique_ptr<SpectrumAnalyser>& ComponentManagement::getSpectrumAnalyser()
{
    if (spectrumAnalyser == nullptr)
    {
        spectrumAnalyser = std::make_unique<SpectrumAnalyser>();
    }
    return spectrumAnalyser;
}

const std::unique_ptr<VectorOscilloscopes>& ComponentManagement::getVectorOscilloscopes()
{
    if (vectorOscilloscopes == nullptr)
    {
        vectorOscilloscopes = std::make_unique<VectorOscilloscopes>();
    }
    return vectorOscilloscopes;
}

const std::unique_ptr<WaveformChartComponent>& ComponentManagement::getWaveformChartComponent()
{
    if (waveformChartComponent == nullptr)
    {
        waveformChartComponent = std::make_unique<WaveformChartComponent>();
    }
    return waveformChartComponent;
}

void ComponentManagement::resetWaveformComponent()
{
    waveformComponent.reset();
}

void ComponentManagement::resetSpectrumAnalyser()
{
    spectrumAnalyser.reset();
}

void ComponentManagement::resetVectorOscilloscopes()
{
    vectorOscilloscopes.reset();
}

void ComponentManagement::resetWaveformChartComponent()
{
    waveformChartComponent.reset();
}

void ComponentManagement::resetAllComponents()
{
    resetWaveformComponent();
    resetSpectrumAnalyser();
    resetVectorOscilloscopes();
    resetWaveformChartComponent();
}