#include "MainComponent.h"

MainComponent::MainComponent() : appState("AppState")
{
    setOpaque(true);

    juce::LookAndFeel::setDefaultLookAndFeel(&lookAndFeel);

    CreateColoursConfiguration& createColoursConfiguration = CreateColoursConfiguration::getInstance();

    auto mainCompCat = createColoursConfiguration.currentColourTheme
        .getChildWithProperty("name", "MainComponent");
    mainComponentBackgroundColour = juce::Colour(createColoursConfiguration.colourHexToARGBInt(
        mainCompCat.getChildWithProperty("name", "Background").getProperty("hex").toString(), false));

    centreWithSize(600,400);
    addAndMakeVisible(header);
    header.WASAPIButton.setBounds(20,10,30,30);
    header.windowsSizeButton.setBounds(60,10,30,30);
    header.headerFixedButton.setBounds(100,10,30,30);
    header.themeConfigurationButton.setBounds(140,10,30,30);
    header.windowControl.setTooltip("Close");
    header.WASAPIButton.setTooltip("Enable or disable miniaudio WASAPI loopback");
    header.windowsSizeButton.setTooltip("Resize Window");
    header.windowsSizeButton.onClick = [this]
        {
            juce::AlertWindow* aw = new juce::AlertWindow(
                "Resize Window",
                "Enter Width and Height,please don't input zero or any number smaller then 200 :)",
                juce::AlertWindow::QuestionIcon);

            aw->setOpaque(false);
            aw->setDropShadowEnabled(false);
            aw->addTextEditor("Width", "", "Width");
            aw->addTextEditor("Height", "", "Height");
            aw->getTextEditor("Width")->setInputRestrictions(4,"0123456789");
            aw->getTextEditor("Height")->setInputRestrictions(4, "0123456789");
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
    header.WASAPIButton.onClick = [this]
        {
            if (header.WASAPIButton.isOpen)
            {
                stopAndCloseWASAPIDevice();
                header.WASAPIButton.isOpen = false;
                header.WASAPIButton.repaint();
            }
            else
            {
                miniAudioWASAPI = std::make_shared<MiniAudioWASAPI>();
                weakMiniAudioWASAPI = miniAudioWASAPI;
                header.WASAPIButton.isOpen = true;
                header.WASAPIButton.repaint();
            }
        };
}

MainComponent::~MainComponent()
{
    pushSampleIntoJuceAudioBuffer.removeAll();
    pushSampleIntoJuceAudioBuffer.removeAllM();
    pushSampleIntoJuceAudioBuffer.removeAllS();
    ComponentManagement::getInstance().resetAllComponents();
    if (auto ptr = weakMiniAudioWASAPI.lock())
    {
        DBG("MainComponent uninit");
        stopAndCloseWASAPIDevice();
    }
}

void MainComponent::paint (juce::Graphics& g)
{
    CreateColoursConfiguration& createColoursConfiguration = CreateColoursConfiguration::getInstance();
    mainComponentBackgroundColour = juce::Colour(createColoursConfiguration.colourHexToARGBInt(
        createColoursConfiguration.currentColourTheme.getChildWithProperty("name", "MainComponentBackground").getProperty("hex").toString(), false));
    g.fillAll(mainComponentBackgroundColour);

    header.setBounds(0, 0, getBounds().getWidth(), 50);
    header.windowControl.setBounds(getBounds().getWidth() - 40, 10, 30, 30);
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
    header.setVisible(true);
}

void MainComponent::mouseExit(const juce::MouseEvent&)
{
    if (!header.isMouseOver() && !header.headerFixedButton.getHeaderFixed())
    {
        header.setVisible(false);
    }
}

void MainComponent::openSpectrumAnalyser(int x, int y)
{
    auto spectrum = ComponentManagement::getInstance().getSpectrumAnalyser();

    if (!spectrum->callbackId)
    {
        spectrum->callbackId = pushSampleIntoJuceAudioBuffer.add(
            [this, spectrum]()
            {
                spectrum->processAudioBuffer(
                    pushSampleIntoJuceAudioBuffer.getLocalAudioBufferReference());
            });
    }

    if (!spectrum->callbackIdS)
    {
        spectrum->callbackIdS = pushSampleIntoJuceAudioBuffer.addS(
            [spectrum]()
            {
                spectrum->callstartTimerHz(60);
            });
    }

    if (!spectrum->callbackIdM)
    {
        spectrum->callbackIdM = pushSampleIntoJuceAudioBuffer.addM(
            [spectrum]()
            {
                spectrum->callStopTimer();
                spectrum->scopeDataReset();
                spectrum->repaint();
            });
    }

    addAndMakeVisible(spectrum.get());
    spectrum->componentControl.setBounds(480, 10, 10, 10);
    spectrum->drawBounds.setBounds(0,0,500,150);
    spectrum->drawBounds.setInterceptsMouseClicks(false, false);

    std::weak_ptr<SpectrumAnalyser> weakSpectrum = spectrum;
    spectrum->componentControl.setCallBackFuntion([this, weakSpectrum]()
        {
            if (auto sp = weakSpectrum.lock())
            {
                pushSampleIntoJuceAudioBuffer.remove(sp->callbackId);
                pushSampleIntoJuceAudioBuffer.removeS(sp->callbackIdS);
                pushSampleIntoJuceAudioBuffer.removeM(sp->callbackIdM);
            }
            ComponentManagement::getInstance().resetSpectrumAnalyser();
        });

    spectrum->setBounds(x, y, 500, 150);
    spectrum->drawArea.setBounds(0, 0, 500, 150);
}

void MainComponent::openWaveformComponent(int x, int y)
{
    auto waveform = ComponentManagement::getInstance().getWaveformComponent();

    if (!waveform->callbackId)
    {
        waveform->callbackId = pushSampleIntoJuceAudioBuffer.add(
            [this, waveform]()
            {
                waveform->drawWaveform(
                    pushSampleIntoJuceAudioBuffer.getLocalAudioBufferReference(),
                    pushSampleIntoJuceAudioBuffer.getLocalAudioBufferRMSReference());
            });
    }

    addAndMakeVisible(waveform.get());
    waveform->componentControl.setBounds(480, 10, 10, 10);
    waveform->drawBounds.setBounds(0, 0, 500, 150);
    waveform->drawBounds.setInterceptsMouseClicks(false, false);

    std::weak_ptr<WaveformComponent> weakWaveform = waveform;
    waveform->componentControl.setCallBackFuntion([this, weakWaveform]()
        {
            if (auto wf = weakWaveform.lock())
            {
                pushSampleIntoJuceAudioBuffer.remove(wf->callbackId);
            }
            ComponentManagement::getInstance().resetWaveformComponent();
        });

    waveform->setBounds(x, y, 500, 150);
    waveform->tileArea.setBounds(0, 0, 16, 150);
    waveform->drawArea.setBounds(0, 0, 500, 150);
}

void MainComponent::openVectorOscilloscopeComponent(int x, int y)
{
    auto vector = ComponentManagement::getInstance().getVectorOscilloscopes();

    if (!vector->callbackId)
    {
        vector->callbackId = pushSampleIntoJuceAudioBuffer.add(
            [this, vector]()
            {
                vector->pushStereoBuffer(
                    pushSampleIntoJuceAudioBuffer.getLocalAudioBufferReadPointer());
                vector->repaint();
            });
    }

    addAndMakeVisible(vector.get());
    vector->componentControl.setBounds(280, 10, 10, 10);
    vector->drawBounds.setBounds(0, 0, 300, 300);
    vector->drawBounds.setInterceptsMouseClicks(false, false);

    std::weak_ptr<VectorOscilloscopes> weakVector = vector;
    vector->componentControl.setCallBackFuntion([this, weakVector]()
        {
            if (auto vec = weakVector.lock())
            {
                pushSampleIntoJuceAudioBuffer.remove(vec->callbackId);
            }
            ComponentManagement::getInstance().resetVectorOscilloscopes();
        });

    vector->setBounds(x, y, 300, 300);
}

void MainComponent::openWaveformChartComponent(int x, int y)
{
    auto chart = ComponentManagement::getInstance().getWaveformChartComponent();

    if (!chart->callbackId)
    {

        chart->callbackId = pushSampleIntoJuceAudioBuffer.add(
            [this, chart]()
            {
                chart->pushStereoBuffer(
                    pushSampleIntoJuceAudioBuffer.getLocalAudioBufferReadPointer(),
                    pushSampleIntoJuceAudioBuffer.getLocalAudioBufferRMS());
                chart->repaint();
            });
    }

    addAndMakeVisible(chart.get());
    chart->componentControl.setBounds(480, 10, 10, 10);
    chart->drawBounds.setBounds(0, 0, 500, 150);
    chart->drawBounds.setInterceptsMouseClicks(false, false);

    std::weak_ptr<WaveformChartComponent> weakChart = chart;
    chart->componentControl.setCallBackFuntion([this, weakChart]()
        {
            if (auto ch = weakChart.lock())
            {
                pushSampleIntoJuceAudioBuffer.remove(ch->callbackId);
            }
            ComponentManagement::getInstance().resetWaveformChartComponent();
        });

    chart->setBounds(x, y, 500, 150);
    chart->drawArea.setBounds(0, 0, 500, 150);
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