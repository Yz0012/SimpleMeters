#include <JuceHeader.h>
#include <windows.h>

#include "MainComponent.h"

class NewProjectApplication : public juce::JUCEApplication
{
public:
    const juce::String getApplicationName() override { return ProjectInfo::projectName; }
    const juce::String getApplicationVersion() override { return ProjectInfo::versionString; }
    bool moreThanOneInstanceAllowed() override { return true; }

    void initialise(const juce::String& commandLine) override
    {
        mainComponent.reset(new MainComponent());
        mainComponent->addToDesktop(juce::ComponentPeer::windowIsResizable | juce::ComponentPeer::windowHasDropShadow);
        mainComponent->setVisible(true);

        juce::MessageManager::callAsync([this] {
            ensureWindowShownInTaskbar(*mainComponent);
            });

    }

    void shutdown() override
    {
        mainComponent = nullptr;
    }

    void systemRequestedQuit() override
    {
        quit();
    }

    void anotherInstanceStarted(const juce::String& commandLine) override {}

    static void ensureWindowShownInTaskbar(juce::Component& comp)
    {
        #if JUCE_WINDOWS
        if (auto* peer = comp.getPeer())
        {
            HWND hwnd = (HWND)peer->getNativeHandle();

            LONG_PTR exStyle = GetWindowLongPtr(hwnd, GWL_EXSTYLE);
            exStyle = (exStyle & ~WS_EX_TOOLWINDOW) | WS_EX_APPWINDOW;
            SetWindowLongPtr(hwnd, GWL_EXSTYLE, exStyle);

            ShowWindow(hwnd, SW_HIDE);
            ShowWindow(hwnd, SW_SHOWNOACTIVATE);

            SetWindowPos(hwnd, nullptr, 0, 0, 0, 0,
                SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER |
                SWP_FRAMECHANGED | SWP_NOACTIVATE);
        }
    #endif
    }

private:
    std::unique_ptr<MainComponent> mainComponent;
};

START_JUCE_APPLICATION(NewProjectApplication)