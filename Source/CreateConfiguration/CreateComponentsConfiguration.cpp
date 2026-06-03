
#include "CreateComponentsConfiguration.h"

CreateComponentsConfiguration::CreateComponentsConfiguration()
{
    currentComponentSettings = loadConfig();
}

CreateComponentsConfiguration::~CreateComponentsConfiguration()
{
}

juce::File CreateComponentsConfiguration::getConfigFile()
{
    auto dir = juce::File::getSpecialLocation(juce::File::userApplicationDataDirectory)
        .getChildFile("SimpleMeters");
    dir.createDirectory();
    return dir.getChildFile("componentsconfig.xml");
}

void CreateComponentsConfiguration::saveConfig(const juce::ValueTree& tree)
{
    auto xml = tree.createXml();
    if (xml != nullptr)
        xml->writeTo(getConfigFile());
}

juce::ValueTree CreateComponentsConfiguration::loadConfig()
{
    juce::File file = getConfigFile();

    if (file.existsAsFile())
    {
        auto xml = juce::parseXML(file);
        if (xml != nullptr)
            return juce::ValueTree::fromXml(*xml);
    }

    return initComponentSettings;
}