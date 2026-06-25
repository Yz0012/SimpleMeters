
#include "StartUpConfiguration.h"

StartUpConfiguration::StartUpConfiguration()
{
    currentConfig = loadConfig();
}

StartUpConfiguration::~StartUpConfiguration()
{
}

/*
* @~chinese 获取默认文件夹下的配置文件
* 
* @return File
*/
juce::File StartUpConfiguration::getConfigFile()
{
    auto dir = juce::File::getSpecialLocation(juce::File::userApplicationDataDirectory)
        .getChildFile("SimpleMeters");
    dir.createDirectory();
    return dir.getChildFile("startupconfig.xml");
}

/*
* @~chinese 保存配置文件到默认文件夹
* 
* @param[in] ValueTree
* @return ValueTree
*/
void StartUpConfiguration::saveConfig(const juce::ValueTree& tree)
{
    auto xml = tree.createXml();
    if (xml != nullptr)
        xml->writeTo(getConfigFile());
}

/*
* @~chinese 将读取到的配置文件转换为ValueTree类型，并返回该ValueTree，若配置损坏或不存在
* 则返回默认配置的ValueTree
* 
* @return ValueTree
*/
juce::ValueTree StartUpConfiguration::loadConfig()
{
    juce::File file = getConfigFile();

    if (file.existsAsFile())
    {
        auto xml = juce::parseXML(file);
        if (xml != nullptr)
            return juce::ValueTree::fromXml(*xml);
    }

    return initConfig;
}

/*
* @~chinese 获取当前的ValueTree，如果当前的ValueTree未初始化，则返回默认ValueTree
*
* @return ValueTree
*/
juce::ValueTree StartUpConfiguration::getCurrentValueTree()
{
    if (!currentConfig.isValid()) return initConfig;
    return currentConfig;
}