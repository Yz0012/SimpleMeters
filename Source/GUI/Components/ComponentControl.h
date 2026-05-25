
#pragma once

#include <JuceHeader.h>

class ComponentControl : public juce::Component
{
public:
	ComponentControl();
	~ComponentControl();

	using Callback = std::function<void()>;

	void paint(juce::Graphics&) override;

	void mouseDown(const juce::MouseEvent& event) override;
	void mouseEnter(const juce::MouseEvent& event) override;

	void setCallBackFuntion(Callback cb);
	void callBackFunction();
private:
	Callback cb = nullptr;

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ComponentControl);
};