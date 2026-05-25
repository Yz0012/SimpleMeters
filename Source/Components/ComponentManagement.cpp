#include "ComponentManagement.h"

ComponentManagement& ComponentManagement::getInstance()
{
    static ComponentManagement instance;
    return instance;
}

std::shared_ptr<WaveformComponent> ComponentManagement::getWaveformComponent()
{
    jassert(juce::MessageManager::getInstance()->isThisTheMessageThread());

    if (waveformComponent == nullptr)
    {
        waveformComponent = std::make_shared<WaveformComponent>();
    }
    return waveformComponent;
}

std::shared_ptr<SpectrumAnalyser> ComponentManagement::getSpectrumAnalyser()
{
    jassert(juce::MessageManager::getInstance()->isThisTheMessageThread());

    if (spectrumAnalyser == nullptr)
    {
        spectrumAnalyser = std::make_shared<SpectrumAnalyser>();
    }
    return spectrumAnalyser;
}

std::shared_ptr<VectorOscilloscopes> ComponentManagement::getVectorOscilloscopes()
{
    jassert(juce::MessageManager::getInstance()->isThisTheMessageThread());

    if (vectorOscilloscopes == nullptr)
    {
        vectorOscilloscopes = std::make_shared<VectorOscilloscopes>();
    }
    return vectorOscilloscopes;
}

std::shared_ptr<WaveformChartComponent> ComponentManagement::getWaveformChartComponent()
{
    jassert(juce::MessageManager::getInstance()->isThisTheMessageThread());

    if (waveformChartComponent == nullptr)
    {
        waveformChartComponent = std::make_shared<WaveformChartComponent>();
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