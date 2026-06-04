#include "MainComponent.h"

MainComponent::MainComponent()
{
    setOpaque(true);

    juce::LookAndFeel::setDefaultLookAndFeel(&lookAndFeel);

    CreateColoursConfiguration& createColoursConfiguration = CreateColoursConfiguration::getInstance();

    mainCategory = createColoursConfiguration.currentColourTheme
        .getChildWithProperty("name", "MainComponent");
    mainComponentBackgroundColour = juce::Colour(createColoursConfiguration.colourHexToARGBInt(
        mainCategory.getChildWithProperty("name", "Background").getProperty("hex").toString(), false));
    mainCategory.addListener(this);

    centreWithSize(600,400);
    addAndMakeVisible(header);
    header.WASAPIButton.setBounds(20, 10, 30, 30);
    header.windowsSizeButton.setBounds(60, 10, 30, 30);
    header.headerFixedButton.setBounds(100, 10, 30, 30);
    header.themeConfigurationButton.setBounds(180, 10, 30, 30);
	header.pinOnTopButton.setBounds(140, 10, 30, 30);
    header.windowControl.setTooltip("Close");
    header.WASAPIButton.setTooltip("Enable or disable miniaudio WASAPI loopback");
    header.windowsSizeButton.setTooltip("Resize Window");
	header.headerFixedButton.setTooltip("Pinned header");
    header.pinOnTopButton.setTooltip("Pinned window");
	header.themeConfigurationButton.setTooltip("Open theme configuration");
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
	header.pinOnTopButton.onClick = [this]
		{
			if (header.pinOnTopButton.pinFixed)
			{
				setAlwaysOnTop(false);
			}
			else
			{
				setAlwaysOnTop(true);
			}
		};
}

MainComponent::~MainComponent()
{
    if (mainCategory.isValid())
        mainCategory.removeListener(this);

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

        menu.addItem(1, "SpectrumAnalyser", true, false, juce::Drawable::createFromImageData(BinaryData::spectrumAnalyzerIcon_png, BinaryData::spectrumAnalyzerIcon_pngSize));
        menu.addItem(2, "Waveform", true, false, juce::Drawable::createFromImageData(BinaryData::WaveformComponent_png, BinaryData::WaveformComponent_pngSize));
        menu.addItem(3, "VectorOscilloscopeComponent", true, false, juce::Drawable::createFromImageData(BinaryData::VectorOscilloscopeComponent_png, BinaryData::VectorOscilloscopeComponent_pngSize));
        menu.addItem(4, "WaveformChart", true, false, juce::Drawable::createFromImageData(BinaryData::WaveformChartComponent_png, BinaryData::WaveformChartComponent_pngSize));

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
    spectrum->componentHeader.componentControl.setBounds(470, 20, 10, 10);
    spectrum->drawBounds.setBounds(0,0,500,150);
    spectrum->drawBounds.setInterceptsMouseClicks(false, false);

    // 不统一
    std::weak_ptr<SpectrumAnalyser> weakSpectrum = spectrum;
    spectrum->componentHeader.componentControl.setCallBackFuntion([this, weakSpectrum]()
        {
            if (auto sp = weakSpectrum.lock())
            {
                pushSampleIntoJuceAudioBuffer.remove(sp->callbackId);
                pushSampleIntoJuceAudioBuffer.removeS(sp->callbackIdS);
                pushSampleIntoJuceAudioBuffer.removeM(sp->callbackIdM);
            }
            ComponentManagement::getInstance().resetSpectrumAnalyser();
        });

    spectrum->cb = [weakSpectrum]()
        {
            juce::PopupMenu menu;

            menu.addItem(1, "Position", true, false);
            menu.addItem(2, "ComponentSize", true, false);
            
            juce::PopupMenu modeMenu;
            modeMenu.addItem(3, "Left", true, false, juce::Drawable::createFromImageData(BinaryData::LAR_png, BinaryData::LAR_pngSize));
            modeMenu.addItem(4, "Right", true, false, juce::Drawable::createFromImageData(BinaryData::LAR_png, BinaryData::LAR_pngSize));
            modeMenu.addItem(5, "Stereo", true, false, juce::Drawable::createFromImageData(BinaryData::LR2_png, BinaryData::LR2_pngSize));
            modeMenu.addItem(6, "LR", true, false, juce::Drawable::createFromImageData(BinaryData::LR_png, BinaryData::LR_pngSize));
            modeMenu.addItem(7, "Interleaved", true, false, juce::Drawable::createFromImageData(BinaryData::LRLRLRLR_png, BinaryData::LRLRLRLR_pngSize));
			menu.addSubMenu("Mode", modeMenu);

            menu.showMenuAsync(
                juce::PopupMenu::Options(),
                [weakSpectrum](int result)
                {
                    if (auto sp = weakSpectrum.lock())
                    {
                        if (result == 0)
                        {
                        }
                        else if (result == 3)
                        {
                            sp->setAnalysisMode(AnalysisMode::Left);
                        }
                        else if (result == 4)
                        {
                            sp->setAnalysisMode(AnalysisMode::Right);
                        }
                        else if (result == 5)
                        {
                            sp->setAnalysisMode(AnalysisMode::Stereo);
                        }
                        else if (result == 6)
                        {
                            sp->setAnalysisMode(AnalysisMode::LR);
                        }
                        else if (result == 7)
                        {
                            sp->setAnalysisMode(AnalysisMode::Interleaved);
                        }
                    }
                });
        };

    spectrum->setBounds(x, y, 500, 150);
    spectrum->drawArea.setBounds(0, 50, 500, 100);
    spectrum->componentHeader.setBounds(0, 0, 500, 50);
    spectrum->componentHeader.themeConfigButton.setBounds(10, 10, 30, 30);
    spectrum->componentHeader.themeConfigButton.setTooltip("Theme Configuration");
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
                waveform->repaint(waveform->getLocalBounds());
            });
    }

    addAndMakeVisible(waveform.get());
    waveform->componentHeader.componentControl.setBounds(470, 20, 10, 10);
    waveform->drawBounds.setBounds(0, 0, 500, 150);
    waveform->drawBounds.setInterceptsMouseClicks(false, false);

    std::weak_ptr<WaveformComponent> weakWaveform = waveform;
    waveform->componentHeader.componentControl.setCallBackFuntion([this, weakWaveform]()
        {
            if (auto wf = weakWaveform.lock())
            {
                pushSampleIntoJuceAudioBuffer.remove(wf->callbackId);
            }
            ComponentManagement::getInstance().resetWaveformComponent();
        });

    waveform->cb = [weakWaveform]()
        {
            juce::PopupMenu menu;
            menu.addItem(1, "Position", true, false);
            menu.addItem(2, "ComponentSize", true, false);
            menu.showMenuAsync(
                juce::PopupMenu::Options(),
                [weakWaveform](int result)
                {
                    if (auto wf = weakWaveform.lock())
                    {
                        if (result == 0)
                        {
                        }
                    }
                });
        };

    waveform->setBounds(x, y, 500, 150);
    waveform->tileArea.setBounds(0, 0, 16, 100);
    waveform->drawArea.setBounds(0, 50, 500, 100);
	waveform->componentHeader.setBounds(0, 0, 500, 50);
	waveform->componentHeader.themeConfigButton.setBounds(10, 10, 30, 30);
    waveform->componentHeader.themeConfigButton.setTooltip("Theme Configuration");
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
                vector->repaint(vector->getLocalBounds());;
            });
    }

    addAndMakeVisible(vector.get());
    vector->componentHeader.componentControl.setBounds(270, 20, 10, 10);
    vector->drawBounds.setBounds(0, 0, 300, 300);
    vector->drawBounds.setInterceptsMouseClicks(false, false);

    std::weak_ptr<VectorOscilloscopes> weakVector = vector;
    vector->componentHeader.componentControl.setCallBackFuntion([this, weakVector]()
        {
            if (auto vec = weakVector.lock())
            {
                pushSampleIntoJuceAudioBuffer.remove(vec->callbackId);
            }
            ComponentManagement::getInstance().resetVectorOscilloscopes();
        });

    vector->cb = [weakVector]()
        {
            juce::PopupMenu menu;
            menu.addItem(1, "Position", true, false);
            menu.addItem(2, "ComponentSize", true, false);
            menu.showMenuAsync(
                juce::PopupMenu::Options(),
                [weakVector](int result)
                {
                    if (auto vec = weakVector.lock())
                    {
                        if (result == 0)
                        {
                        }
                    }
                });
        };

    vector->setBounds(x, y, 300, 300);
    vector->componentHeader.setBounds(0, 0, 300, 50);
    vector->componentHeader.themeConfigButton.setBounds(10, 10, 30, 30);
    vector->componentHeader.themeConfigButton.setTooltip("Theme Configuration");
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
                chart->repaint(chart->getLocalBounds());
            });
    }

    addAndMakeVisible(chart.get());
    chart->componentHeader.componentControl.setBounds(470, 20, 10, 10);
    chart->drawBounds.setBounds(0, 0, 500, 150);
    chart->drawBounds.setInterceptsMouseClicks(false, false);

    std::weak_ptr<WaveformChartComponent> weakChart = chart;
    chart->componentHeader.componentControl.setCallBackFuntion([this, weakChart]()
        {
            if (auto ch = weakChart.lock())
            {
                pushSampleIntoJuceAudioBuffer.remove(ch->callbackId);
            }
            ComponentManagement::getInstance().resetWaveformChartComponent();
        });

    chart->cb = [weakChart]()
        {
            juce::PopupMenu menu;
            menu.addItem(1, "Position", true, false);
            menu.addItem(2, "ComponentSize", true, false);
            menu.showMenuAsync(
                juce::PopupMenu::Options(),
                [weakChart](int result)
                {
                    if (auto ch = weakChart.lock())
                    {
                        if (result == 0)
                        {
                        }
                    }
                });
        };

    chart->setBounds(x, y, 500, 150);
    chart->drawArea.setBounds(0, 50, 500, 100);
    chart->componentHeader.setBounds(0, 0, 500, 50);
    chart->componentHeader.themeConfigButton.setBounds(10, 10, 30, 30);
    chart->componentHeader.themeConfigButton.setTooltip("Theme Configuration");
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

void MainComponent::valueTreePropertyChanged(juce::ValueTree& tree, const juce::Identifier& property)
{
    if (property == juce::Identifier("hex"))
    {
        this->mainComponentBackgroundColour = juce::Colour(CreateColoursConfiguration::getInstance().colourHexToARGBInt(
            mainCategory.getChildWithProperty("name", "Background").getProperty("hex").toString(), false));
		repaint();
    }
}