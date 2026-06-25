
#include "CreateColoursConfiguration.h"

CreateColoursConfiguration::CreateColoursConfiguration()
{
	currentColourTheme = loadConfig();
};
CreateColoursConfiguration::~CreateColoursConfiguration() {};

juce::uint32 CreateColoursConfiguration::colourHexToARGBInt(const juce::String& hexString, bool isRRGGBBAA)
{
    juce::String clean = hexString.trim();
    if (clean.startsWith("#") || clean.startsWith("0x") || clean.startsWith("$"))
        clean = clean.substring(1);
    clean = clean.toUpperCase();

    if (clean.length() == 3)
    {
        juce::uint8 r = (clean[0] <= '9' ? clean[0] - '0' : clean[0] - '7') * 0x11;
        juce::uint8 g = (clean[1] <= '9' ? clean[1] - '0' : clean[1] - '7') * 0x11;
        juce::uint8 b = (clean[2] <= '9' ? clean[2] - '0' : clean[2] - '7') * 0x11;
        return 0xFF000000 | (r << 16) | (g << 8) | b;
    }
    else if (clean.length() == 4)
    {
        juce::uint8 a = (clean[0] <= '9' ? clean[0] - '0' : clean[0] - '7') * 0x11;
        juce::uint8 r = (clean[1] <= '9' ? clean[1] - '0' : clean[1] - '7') * 0x11;
        juce::uint8 g = (clean[2] <= '9' ? clean[2] - '0' : clean[2] - '7') * 0x11;
        juce::uint8 b = (clean[3] <= '9' ? clean[3] - '0' : clean[3] - '7') * 0x11;
        return (a << 24) | (r << 16) | (g << 8) | b;
    }
    else if (clean.length() == 6)
    {
        juce::uint32 rgb = clean.getHexValue32();
        return 0xFF000000 | rgb;
    }
    else if (clean.length() == 8)
    {
        if (isRRGGBBAA)
        {
            juce::String rrggbb = clean.substring(0, 6);
            juce::String aa = clean.substring(6);
            juce::String aarrggbb = aa + rrggbb;
            return aarrggbb.getHexValue32();
        }
        else
        {
            return clean.getHexValue32();
        }
    }
    else
    {
        jassertfalse;
        return 0xFF000000;
    }
}

/*
* @~chinese 获取默认文件夹下的配置文件
*
* @return File
*/
juce::File CreateColoursConfiguration::getConfigFile()
{
    auto dir = juce::File::getSpecialLocation(juce::File::userApplicationDataDirectory)
        .getChildFile("SimpleMeters");
    dir.createDirectory();
    return dir.getChildFile("coloursconfig.xml");
}

/*
* @~chinese 保存配置文件到默认文件夹
*
* @param[in] ValueTree
* @return ValueTree
*/
void CreateColoursConfiguration::saveConfig(const juce::ValueTree& tree)
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
juce::ValueTree CreateColoursConfiguration::loadConfig()
{
    juce::File file = getConfigFile();

    if (file.existsAsFile())
    {
        auto xml = juce::parseXML(file);
        if (xml != nullptr)
            return juce::ValueTree::fromXml(*xml);
    }

    return initColourTheme;
}

/*
* @~chinese 获取当前的ValueTree，如果当前的ValueTree未初始化，则返回默认ValueTree
* 
* @return ValueTree
*/
juce::ValueTree CreateColoursConfiguration::getCurrentValueTree()
{
    if (!currentColourTheme.isValid()) return initColourTheme;
    return currentColourTheme;
}