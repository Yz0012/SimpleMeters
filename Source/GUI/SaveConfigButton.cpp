
#include "SaveConfigButton.h"

SaveConfigButton::SaveConfigButton()
{
}

SaveConfigButton::~SaveConfigButton()
{
	/*
	* @~chinese 在这里我之所以不用构造函数初始化列表，是因为ValueTree单例先于 SaveConfigButton析构，
	导致SaveConfigButton在~SaveConfigButton中调用removeListener时访问了已销毁的内存
	*/
	//currentConfig.removeListener(this);
}

void SaveConfigButton::paint(juce::Graphics& g)
{
	/*
	* @~chinese 我本想在这里使用一种能去掉if语句的方法，就是将image拷贝到成员当中，再由valueTreePropertyChanged方法调用
	但我考虑一会后发现这种方法并不实惠，虽然也是一种可行的结构
	*/
	if (configSaved)
	{
		g.drawImageAt(juce::ImageCache::getFromMemory(BinaryData::SaveConfigIcon_png, BinaryData::SaveConfigIcon_pngSize), 0, 0);
	}
	else
	{
		g.drawImageAt(juce::ImageCache::getFromMemory(BinaryData::SaveConfigIconTwo_png, BinaryData::SaveConfigIconTwo_pngSize), 0, 0);
	}
}

void SaveConfigButton::mouseDown(const juce::MouseEvent& event)
{
	// 这里需要筛除掉已保存的状态
	if (configSaved) return;
	configSaved = !configSaved;
	repaint();
	StartUpConfiguration::getInstance().saveConfig(StartUpConfiguration::getInstance().getCurrentValueTree());
}

void SaveConfigButton::mouseEnter(const juce::MouseEvent&)
{
	// 已保存不需要改变指针图标
	if (!configSaved)
	{
		setMouseCursor(juce::MouseCursor::PointingHandCursor);
	}
}

/*
* @~chinese 当ValueTree改变，这个方法会被调用并将改变当前组件图标
* @see MainComponent
*/
void SaveConfigButton::valueTreePropertyChanged()
{
	// 这里需要筛除掉未保存的状态
	if (!configSaved) return;
	DBG("ConfigButton icon was changed.");
	configSaved = !configSaved;
	repaint();
}