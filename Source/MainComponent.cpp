#include "MainComponent.h"

MainComponent::MainComponent()
{
    setOpaque(true);

    lookAndFeel = std::make_unique<LookAndFeel>();
    header = std::make_unique<Header>();

    juce::LookAndFeel::setDefaultLookAndFeel(lookAndFeel.get());

    createColoursConfiguration = CreateColoursConfiguration::getInstance();

    mainComponentBackgroundColour = juce::Colour(createColoursConfiguration->colourHexToARGBInt(
        createColoursConfiguration->currentColourTheme.getChildWithProperty("name", "MainComponentBackground").getProperty("hex").toString(), false));

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
                        if (Width <= 200 && Height <= 200) return;
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

        menu.showMenuAsync(
            juce::PopupMenu::Options(),
            [this,event](int result)
            {
                if (result == 0) {
                }
                else if (result == 1) {
					openSpectrumAnalyser(event.getMouseDownX(), event.getMouseDownY());
                }
                else if (result == 2) {
                    openWaveformComponent(event.getMouseDownX(), event.getMouseDownY());
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
    repaint();
}

void MainComponent::mouseExit(const juce::MouseEvent&)
{
    if (!header->isMouseOver())
    {
        header->setVisible(false);
        repaint();
    }
}

void MainComponent::openSpectrumAnalyser(int x, int y)
{
    if(auto ptr = weakMiniAudioWASAPI.lock())
    {
        if (!(ptr->getSpectrumAnalyser()->callbackId))
        {
            ptr->getSpectrumAnalyser()->callbackId = ptr->pushSampleIntoJuceAudioBuffer->addR([ptr]() {
                ptr->getSpectrumAnalyser()->pushNextSampleIntoFifo(ptr->pushSampleIntoJuceAudioBuffer->sample);
                });
        }
        if (!(ptr->getSpectrumAnalyser()->callbackIdS))
        {
            ptr->getSpectrumAnalyser()->callbackIdS = ptr->pushSampleIntoJuceAudioBuffer->addS([ptr]() {
                ptr->getSpectrumAnalyser()->callstartTimerHz(60);
                });
        }
        if (!(ptr->getSpectrumAnalyser()->callbackIdM))
        {
            ptr->getSpectrumAnalyser()->callbackIdM = ptr->pushSampleIntoJuceAudioBuffer->addM([ptr]() {
                ptr->getSpectrumAnalyser()->callStopTimer();
                });
        }
    }
    addAndMakeVisible(ComponentManagement::getInstance()->getSpectrumAnalyser().get());
    ComponentManagement::getInstance()->getSpectrumAnalyser().get()->setBounds(x, y, 500, 150);
    ComponentManagement::getInstance()->getSpectrumAnalyser().get()->drawArea.setBounds(0, 0, 500, 150);
}

void MainComponent::openWaveformComponent(int x, int y)
{
	if (auto ptr = weakMiniAudioWASAPI.lock())
	{
        if (!(ptr->getWaveformComponent()->callbackId))
        {
            ptr->getWaveformComponent()->callbackId = ptr->pushSampleIntoJuceAudioBuffer->add([ptr]() {
                ptr->getWaveformComponent()->localAudioBuffer = ptr->pushSampleIntoJuceAudioBuffer->getLocalAudioBufferReadPointer();
                ptr->getWaveformComponent()->localAudioBufferRMS = ptr->pushSampleIntoJuceAudioBuffer->getLocalAudioBufferRMSReadPointer<float>();
                ptr->getWaveformComponent()->drawWaveform();
                });
        }
	}
    addAndMakeVisible(ComponentManagement::getInstance()->getWaveformComponent().get());
    ComponentManagement::getInstance()->getWaveformComponent().get()->setBounds(x, y, 500, 150);
    ComponentManagement::getInstance()->getWaveformComponent().get()->tileArea.setBounds(0, 0, 16, 150);
    ComponentManagement::getInstance()->getWaveformComponent().get()->drawArea.setBounds(0, 0, 500, 150);
}

void MainComponent::userTriedToCloseWindow()
{
    juce::JUCEApplication::getInstance()->systemRequestedQuit();
}

void MainComponent::stopAndCloseWASAPIDevice()
{
    //ComponentManagement::getInstance()->resetAllComponents();
    miniAudioWASAPI->stopDevice();
    DBG("device stop");
    miniAudioWASAPI->pushSampleIntoJuceAudioBuffer->removeAll();
    miniAudioWASAPI->pushSampleIntoJuceAudioBuffer->removeAllR();
    miniAudioWASAPI->pushSampleIntoJuceAudioBuffer->removeAllM();
    miniAudioWASAPI->pushSampleIntoJuceAudioBuffer->removeAllS();
    miniAudioWASAPI.reset();
    DBG("miniAudioReset");
}

void MainComponent::stopWASAPIDevice()
{
    miniAudioWASAPI->stopDevice();
    DBG("device stop");
}