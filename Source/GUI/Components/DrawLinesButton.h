
#pragma once

#include <JuceHeader.h>

class DrawLinesButton : public juce::Component
{
	using Callback = std::function<void()>;
public:
	DrawLinesButton();
	~DrawLinesButton();

	void paint(juce::Graphics&) override;

	void mouseDown(const juce::MouseEvent& event) override;
	void mouseEnter(const juce::MouseEvent& event) override;

	void callbackFunction() const;

	Callback onClick;

	bool isOpen = false;
private:
	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(DrawLinesButton)
};