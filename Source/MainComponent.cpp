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
    header->WASAPIButton.setTooltip("Enable or disable miniaudio WASAPI loopback");
    header->WASAPIButton.onClick = [this]
        {
            if (header->WASAPIButton.isOpen)
            {
                miniAudioWASAPI->stopDevice();
                miniAudioWASAPI->pushSampleIntoJuceAudioBuffer->removeAll();
                miniAudioWASAPI->pushSampleIntoJuceAudioBuffer->removeAllR();
                miniAudioWASAPI->pushSampleIntoJuceAudioBuffer->removeAllM();
                miniAudioWASAPI->pushSampleIntoJuceAudioBuffer->removeAllS();
                miniAudioWASAPI.reset();
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
        miniAudioWASAPI->pushSampleIntoJuceAudioBuffer->removeAll();
        miniAudioWASAPI->pushSampleIntoJuceAudioBuffer->removeAllR();
        miniAudioWASAPI->pushSampleIntoJuceAudioBuffer->removeAllM();
        miniAudioWASAPI->pushSampleIntoJuceAudioBuffer->removeAllS();
        miniAudioWASAPI.reset();
        DBG("miniAudioReset");
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
        addAndMakeVisible(ptr->getSpectrumAnalyser());
		ptr->getSpectrumAnalyser()->setBounds(x, y, 500, 150);
        ptr->getSpectrumAnalyser()->drawArea.setBounds(0,0,500,150);
    }
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
		addAndMakeVisible(ptr->getWaveformComponent());
		ptr->getWaveformComponent()->setBounds(x, y, 500, 150);
		ptr->getWaveformComponent()->tileArea.setBounds(0, 0, 16, 150);
        ptr->getWaveformComponent()->drawArea.setBounds(0, 0, 500, 150);
	}
}

void MainComponent::userTriedToCloseWindow()
{
    juce::JUCEApplication::getInstance()->systemRequestedQuit();
}