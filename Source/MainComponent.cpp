#include "MainComponent.h"

MainComponent::MainComponent()
{
    setOpaque(true);

    lookAndFeel = std::make_unique<LookAndFeel>();
    header = std::make_unique<Header>();

    juce::LookAndFeel::setDefaultLookAndFeel(lookAndFeel.get());

    CreateColoursConfiguration& createColoursConfiguration = CreateColoursConfiguration::getInstance();

    mainComponentBackgroundColour = juce::Colour(createColoursConfiguration.colourHexToARGBInt(
        createColoursConfiguration.currentColourTheme.getChildWithProperty("name", "MainComponentBackground").getProperty("hex").toString(), false));

    centreWithSize(600,400);
    addAndMakeVisible(header.get());
    header->WASAPIButton.setBounds(20,10,30,30);
    header->windowsSizeButton.setBounds(60,10,30,30);
    header->windowControl.setTooltip("Close");
    header->WASAPIButton.setTooltip("Enable or disable miniaudio WASAPI loopback");
    header->windowsSizeButton.setTooltip("Resize Window");
    header->windowsSizeButton.onClick = [this]
        {
            juce::AlertWindow* aw = new juce::AlertWindow(
                "Resize Window",
                "Enter Width and Height,please don't input zero or any number smaller then 200 :)",
                juce::AlertWindow::QuestionIcon);

            aw->addTextEditor("Width", "", "Width");
            aw->addTextEditor("Height", "", "Height");
            aw->getTextEditor("Width")->setInputRestrictions(5,"0123456789");
            aw->getTextEditor("Height")->setInputRestrictions(5, "0123456789");
            aw->addButton("OK", 1, juce::KeyPress(juce::KeyPress::returnKey));
            aw->addButton("Cancel", 0, juce::KeyPress(juce::KeyPress::escapeKey));

            aw->enterModalState(true, juce::ModalCallbackFunction::create(
                [this,aw](int result)
                {
                    if (result == 1)
                    {
                        int &&Width = aw->getTextEditorContents("Width").getIntValue();
                        int &&Height = aw->getTextEditorContents("Height").getIntValue();
                        if (Width <= 200 || Height <= 200) return;
                        setSize(aw->getTextEditorContents("Width").getIntValue(), aw->getTextEditorContents("Height").getIntValue());
                    }
                    delete aw;
                }
            ), true);
        };
    header->WASAPIButton.onClick = [this]
        {
            if (header->WASAPIButton.isOpen)
            {
                stopAndCloseWASAPIDevice();
                header->WASAPIButton.isOpen = false;
                header->WASAPIButton.repaint();
            }
            else
            {
                miniAudioWASAPI = std::make_shared<MiniAudioWASAPI>();
                weakMiniAudioWASAPI = miniAudioWASAPI;
                header->WASAPIButton.isOpen = true;
                header->WASAPIButton.repaint();
            }
        };
}

MainComponent::~MainComponent()
{
    ComponentManagement::getInstance().resetAllComponents();
    if (auto ptr = weakMiniAudioWASAPI.lock())
    {
        DBG("MainComponent uninit");
        stopAndCloseWASAPIDevice();
    }
}

void MainComponent::paint (juce::Graphics& g)
{
    g.fillAll(mainComponentBackgroundColour);

    header->setBounds(0, 0, getBounds().getWidth(), 50);
    header->windowControl.setBounds(getBounds().getWidth() - 40, 10, 30, 30);
}

void MainComponent::resized()
{
}

void MainComponent::mouseDown(const juce::MouseEvent& event)
{
    if (event.mods.isLeftButtonDown())
    {
        dragger.startDraggingComponent(this, event);
    }
    if (event.mods.isPopupMenu())
    {
        juce::PopupMenu menu;

        menu.addItem(1, "SpectrumAnalyser");
        menu.addItem(2, "Waveform");
        menu.addItem(3, "VectorOscilloscopeComponent");
        menu.addItem(4, "WaveformChart");

        menu.showMenuAsync(
            juce::PopupMenu::Options(),
            [this,event](int result)
            {
                if (result == 0)
                {
                }
                else if (result == 1)
                {
					openSpectrumAnalyser(event.getMouseDownX(), event.getMouseDownY());
                }
                else if (result == 2)
                {
                    openWaveformComponent(event.getMouseDownX(), event.getMouseDownY());
                }
                else if (result == 3)
                {
                    openVectorOscilloscopeComponent(event.getMouseDownX(), event.getMouseDownY());
                }
                else if (result == 4)
                {
                    openWaveformChartComponent(event.getMouseDownX(), event.getMouseDownY());
                }
            });
    }
}

void MainComponent::mouseDrag(const juce::MouseEvent& event)
{
	dragger.dragComponent(this, event, nullptr);
}

void MainComponent::mouseEnter(const juce::MouseEvent&)
{
    setMouseCursor(juce::MouseCursor::DraggingHandCursor);
    header->setVisible(true);
}

void MainComponent::mouseExit(const juce::MouseEvent&)
{
    if (!header->isMouseOver())
    {
        header->setVisible(false);
    }
}

void MainComponent::openSpectrumAnalyser(int x, int y)
{
    if (!(ComponentManagement::getInstance().getSpectrumAnalyser()->callbackId))
    {
        ComponentManagement::getInstance().getSpectrumAnalyser()->callbackId = pushSampleIntoJuceAudioBuffer.add([this]() {
            ComponentManagement::getInstance().getSpectrumAnalyser()->processAudioBuffer(pushSampleIntoJuceAudioBuffer.getLocalAudioBufferReference());
            });
    }
    if (!(ComponentManagement::getInstance().getSpectrumAnalyser()->callbackIdS))
    {
        ComponentManagement::getInstance().getSpectrumAnalyser()->callbackIdS = pushSampleIntoJuceAudioBuffer.addS([this]() {
            juce::MessageManager::callAsync([]()
                {
                    ComponentManagement::getInstance().getSpectrumAnalyser()->callstartTimerHz(60);
                });
            });
    }
    if (!(ComponentManagement::getInstance().getSpectrumAnalyser()->callbackIdM))
    {
        ComponentManagement::getInstance().getSpectrumAnalyser()->callbackIdM = pushSampleIntoJuceAudioBuffer.addM([this]() {
            juce::MessageManager::callAsync([]()
                {
                    ComponentManagement::getInstance().getSpectrumAnalyser()->callStopTimer();
                    ComponentManagement::getInstance().getSpectrumAnalyser()->scopeDataReset();
                    ComponentManagement::getInstance().getSpectrumAnalyser()->repaint();
                });
            });
    }
    addAndMakeVisible(ComponentManagement::getInstance().getSpectrumAnalyser().get());
    ComponentManagement::getInstance().getSpectrumAnalyser()->setBounds(x, y, 500, 150);
    ComponentManagement::getInstance().getSpectrumAnalyser()->drawArea.setBounds(0, 0, 500, 150);
}

void MainComponent::openWaveformComponent(int x, int y)
{
    if (!(ComponentManagement::getInstance().getWaveformComponent()->callbackId))
    {
        ComponentManagement::getInstance().getWaveformComponent()->callbackId = pushSampleIntoJuceAudioBuffer.add([this]() {
            juce::MessageManager::callAsync([this]() {
                ComponentManagement::getInstance().getWaveformComponent()->drawWaveform(pushSampleIntoJuceAudioBuffer.getLocalAudioBufferReference(), pushSampleIntoJuceAudioBuffer.getLocalAudioBufferRMSReference<float>());
                });
            });
    }
    addAndMakeVisible(ComponentManagement::getInstance().getWaveformComponent().get());
    ComponentManagement::getInstance().getWaveformComponent()->setBounds(x, y, 500, 150);
    ComponentManagement::getInstance().getWaveformComponent()->tileArea.setBounds(0, 0, 16, 150);
    ComponentManagement::getInstance().getWaveformComponent()->drawArea.setBounds(0, 0, 500, 150);
}

void MainComponent::openVectorOscilloscopeComponent(int x, int y)
{
    if (!(ComponentManagement::getInstance().getVectorOscilloscopes()->callbackId))
    {
        ComponentManagement::getInstance().getVectorOscilloscopes()->pushStereoBuffer(pushSampleIntoJuceAudioBuffer.getLocalAudioBufferReadPointer());
        ComponentManagement::getInstance().getVectorOscilloscopes()->callbackId = pushSampleIntoJuceAudioBuffer.add([this]() {
            juce::MessageManager::callAsync([]() {
                ComponentManagement::getInstance().getVectorOscilloscopes()->repaint();
                });
            });
    }
    addAndMakeVisible(ComponentManagement::getInstance().getVectorOscilloscopes().get());
    ComponentManagement::getInstance().getVectorOscilloscopes()->setBounds(x, y, 300, 300);
}

void MainComponent::openWaveformChartComponent(int x, int y)
{
    if (!(ComponentManagement::getInstance().getWaveformChartComponent()->callbackId))
    {
        ComponentManagement::getInstance().getWaveformChartComponent()->pushStereoBuffer(pushSampleIntoJuceAudioBuffer.getLocalAudioBufferReadPointer(), pushSampleIntoJuceAudioBuffer.getLocalAudioBufferRMSReadPointer<float>());
        ComponentManagement::getInstance().getWaveformChartComponent()->callbackId = pushSampleIntoJuceAudioBuffer.add([this]() {
            juce::MessageManager::callAsync([]() {
                ComponentManagement::getInstance().getWaveformChartComponent()->repaint();
                });
            });
    }
    addAndMakeVisible(ComponentManagement::getInstance().getWaveformChartComponent().get());
    ComponentManagement::getInstance().getWaveformChartComponent()->setBounds(x, y, 500, 150);
    ComponentManagement::getInstance().getWaveformChartComponent()->drawArea.setBounds(0, 0, 500, 150);
}

void MainComponent::userTriedToCloseWindow()
{
    juce::JUCEApplication::getInstance()->systemRequestedQuit();
}

void MainComponent::stopAndCloseWASAPIDevice()
{
    miniAudioWASAPI->stopDevice();
    DBG("device stop");
    miniAudioWASAPI.reset();
    DBG("miniAudioReset");
}

void MainComponent::stopWASAPIDevice()
{
    miniAudioWASAPI->stopDevice();
    DBG("device stop");
}