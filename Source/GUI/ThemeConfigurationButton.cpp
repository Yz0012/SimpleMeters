
#include "ThemeConfigurationButton.h"

ThemeConfigurationButton::ThemeConfigurationButton()
{
    CreateColoursConfiguration& createColoursConfiguration = CreateColoursConfiguration::getInstance();
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
    // 1. 创建编辑器组件并放入 Viewport
    auto* editor = new ColourThemeEditor(createColoursConfiguration.currentColourTheme);
    auto* viewport = new juce::Viewport();
    viewport->setViewedComponent(editor, false);
    viewport->setScrollBarsShown(true, false);

    int prefWidth = 500;
    int prefHeight = juce::jmin(editor->getRequiredHeight(), 400);
    viewport->setSize(prefWidth, prefHeight);
    editor->setSize(prefWidth, editor->getRequiredHeight());

    // 2. 创建用于对话框的顶层组件（包含 Viewport + OK/Cancel 按钮）
    class DialogContent : public juce::Component {
    public:
        DialogContent(ColourThemeEditor* editor, juce::ValueTree& themeToEdit)
            : theme(themeToEdit), editorComp(editor), viewport(editor->getParentComponent())
        {
            // 添加按钮
            okButton.onClick = [this] {
                editorComp->applyChanges();   // 先保存修改
                if (auto* dw = findParentComponentOfClass<juce::DialogWindow>())
                    dw->exitModalState(1);    // 返回 1 表示 OK
                };
            cancelButton.onClick = [this] {
                if (auto* dw = findParentComponentOfClass<juce::DialogWindow>())
                    dw->exitModalState(0);    // 返回 0 表示 Cancel
                };

            addAndMakeVisible(viewport);
            addAndMakeVisible(okButton);
            addAndMakeVisible(cancelButton);
        }

        void resized() override {
            auto area = getLocalBounds();
            auto buttonArea = area.removeFromBottom(40);
            okButton.setBounds(buttonArea.removeFromRight(80).reduced(4, 4));
            cancelButton.setBounds(buttonArea.removeFromRight(80).reduced(4, 4));
            viewport->setBounds(area);
        }

    private:
        juce::ValueTree& theme;
        ColourThemeEditor* editorComp;
        juce::Component* viewport;
        juce::TextButton okButton{ "OK" }, cancelButton{ "Cancel" };
    };

    // 3. 设置 LaunchOptions
    juce::DialogWindow::LaunchOptions options;
    options.dialogTitle = "Edit Colour Theme";
    options.dialogBackgroundColour = juce::Colours::greenyellow;
    options.escapeKeyTriggersCloseButton = true;     // ESC 键关闭
    options.useNativeTitleBar = false;               // 使用自定义标题栏
    options.resizable = true;
    options.useBottomRightCornerResizer = true;
    options.componentToCentreAround = this;          // 居中显示

    // 创建内容组件
    auto* content = new DialogContent(editor, createColoursConfiguration.currentColourTheme);
    content->setSize(520, 400);
    options.content.setOwned(content);               // 传递所有权

    // 4. 启动模态对话框，异步运行
    auto* dialog = options.launchAsync();

    // 附加回调，等待对话框关闭
    juce::ModalComponentManager::getInstance()->attachCallback(
        dialog,
        juce::ModalCallbackFunction::create(
            [this, editor](int result)
            {
                if (result == 1)    // OK 被点击
                {
                    // 修改已经由 applyChanges 完成
                    // 刷新 UI
                    repaint();
                }
                // 对话框会在回调结束后自动销毁
            }
        )
    );
}

void ThemeConfigurationButton::mouseEnter(const juce::MouseEvent&)
{
	setMouseCursor(juce::MouseCursor::PointingHandCursor);
}