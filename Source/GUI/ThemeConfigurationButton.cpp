
#include "ThemeConfigurationButton.h"

ThemeConfigurationButton::ThemeConfigurationButton()
{
}

ThemeConfigurationButton::~ThemeConfigurationButton()
{
}

void ThemeConfigurationButton::paint(juce::Graphics& g)
{
	g.drawImageAt(juce::ImageCache::getFromMemory(BinaryData::themeConfigurationIcon_png, BinaryData::themeConfigurationIcon_pngSize), 0, 0);
}

void ThemeConfigurationButton::mouseDown(const juce::MouseEvent& event)
{
    CreateColoursConfiguration& createColoursConfiguration = CreateColoursConfiguration::getInstance();

    juce::ValueTree mainCategory = createColoursConfiguration.getCurrentValueTree()
        .getChildWithProperty("name", "MainComponent");

    if (!mainCategory.isValid())
    {
        juce::AlertWindow::showMessageBoxAsync(juce::AlertWindow::WarningIcon,
            "Error",
            "Cannot find 'MainComponent' category.");
        return;
    }

    class DialogContent : public juce::Component
    {
    public:
        DialogContent(juce::ValueTree categoryNode)
            : category(std::move(categoryNode))
        {
            auto editor = std::make_unique<ColourThemeEditor>(category);
            int prefWidth = 500;
            int prefHeight = juce::jmin(editor->getRequiredHeight(), 400);
            editor->setSize(prefWidth, editor->getRequiredHeight());

            viewport = std::make_unique<juce::Viewport>();
            viewport->setViewedComponent(editor.release(), true);
            viewport->setScrollBarsShown(true, false);
            viewport->setSize(prefWidth, prefHeight);
            addAndMakeVisible(viewport.get());

            addAndMakeVisible(okButton);
            addAndMakeVisible(cancelButton);

            okButton.onClick = [this] {
                if (auto* ed = dynamic_cast<ColourThemeEditor*>(viewport->getViewedComponent()))
                    ed->applyChanges();
                if (auto* dw = findParentComponentOfClass<juce::DialogWindow>())
                    dw->exitModalState(1);
                };

            cancelButton.onClick = [this] {
                if (auto* dw = findParentComponentOfClass<juce::DialogWindow>())
                    dw->exitModalState(0);
                };

            setSize(520, prefHeight + 60);
        }

        void resized() override
        {
            auto area = getLocalBounds();
            auto buttonArea = area.removeFromBottom(40);
            okButton.setBounds(buttonArea.removeFromRight(80).reduced(4, 4));
            cancelButton.setBounds(buttonArea.removeFromRight(80).reduced(4, 4));
            if (viewport) viewport->setBounds(area);
        }

    private:
        juce::ValueTree category;
        std::unique_ptr<juce::Viewport> viewport;
        juce::TextButton okButton{ "OK" }, cancelButton{ "Cancel" };
    };

    auto content = std::make_unique<DialogContent>(mainCategory);

    juce::DialogWindow::LaunchOptions options;
    options.dialogTitle = "Edit MainComponent Colours(Does't support transparency :)";
    options.dialogBackgroundColour = juce::Colour(0xFF172027);
    options.escapeKeyTriggersCloseButton = true;
    options.useNativeTitleBar = false;
    options.resizable = true;
    options.useBottomRightCornerResizer = true;
    options.componentToCentreAround = this;
    options.content.setOwned(content.release());

    auto* dialog = options.launchAsync();

    juce::ModalComponentManager::getInstance()->attachCallback(
        dialog,
        juce::ModalCallbackFunction::create([this](int result) {
            //
            })
    );
}


void ThemeConfigurationButton::mouseEnter(const juce::MouseEvent&)
{
	setMouseCursor(juce::MouseCursor::PointingHandCursor);
}