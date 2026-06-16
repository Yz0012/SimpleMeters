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
            auto aw = std::make_unique<juce::AlertWindow>(
                "Resize Window",
                "Enter Width and Height, please don't input zero or any number smaller than 200 :)",
                juce::AlertWindow::QuestionIcon);

            aw->setOpaque(false);
            aw->setDropShadowEnabled(false);
            aw->addTextEditor("Width", "", "Width");
            aw->addTextEditor("Height", "", "Height");
            aw->getTextEditor("Width")->setInputRestrictions(4, "0123456789");
            aw->getTextEditor("Height")->setInputRestrictions(4, "0123456789");
            aw->addButton("OK", 1, juce::KeyPress(juce::KeyPress::returnKey));
            aw->addButton("Cancel", 0, juce::KeyPress(juce::KeyPress::escapeKey));

            juce::AlertWindow* rawPtr = aw.release();

            rawPtr->enterModalState(true, juce::ModalCallbackFunction::create(
                [this, rawPtr](int result)
                {
                    if (result == 1)
                    {
                        int width = rawPtr->getTextEditorContents("Width").getIntValue();
                        int height = rawPtr->getTextEditorContents("Height").getIntValue();
                        if (width > 200 && height > 200)
                            setSize(width, height);
                    }
                    delete rawPtr;
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
        menu.addItem(2, "SpectrumAnalyser(Mono)", true, false, juce::Drawable::createFromImageData(BinaryData::LRMono_png, BinaryData::LRMono_pngSize));
        menu.addItem(3, "Waveform", true, false, juce::Drawable::createFromImageData(BinaryData::WaveformComponent_png, BinaryData::WaveformComponent_pngSize));
        menu.addItem(4, "VectorOscilloscopeComponent", true, false, juce::Drawable::createFromImageData(BinaryData::VectorOscilloscopeComponent_png, BinaryData::VectorOscilloscopeComponent_pngSize));
        menu.addItem(5, "WaveformChart", true, false, juce::Drawable::createFromImageData(BinaryData::WaveformChartComponent_png, BinaryData::WaveformChartComponent_pngSize));
        menu.addItem(6, "RMSMeter", true, false, juce::Drawable::createFromImageData(BinaryData::RMSComponent_png, BinaryData::RMSComponent_pngSize));

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
                    openSpectrumAnalyserMono(event.getMouseDownX(), event.getMouseDownY());
                }
                else if (result == 3)
                {
                    openWaveformComponent(event.getMouseDownX(), event.getMouseDownY());
                }
                else if (result == 4)
                {
                    openVectorOscilloscopeComponent(event.getMouseDownX(), event.getMouseDownY());
                }
                else if (result == 5)
                {
                    openWaveformChartComponent(event.getMouseDownX(), event.getMouseDownY());
                }
                else if (result == 6)
                {
					openRMSMeterComponent(event.getMouseDownX(), event.getMouseDownY());
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

            menu.addItem(1, "Position", true, false, juce::Drawable::createFromImageData(BinaryData::menuPosition_png, BinaryData::menuPosition_pngSize));
            menu.addItem(2, "ComponentSize", true, false, juce::Drawable::createFromImageData(BinaryData::menuComponentSize_png, BinaryData::menuComponentSize_pngSize));
            
            juce::PopupMenu modeMenu;
            modeMenu.addItem(3, "Left", true, false, juce::Drawable::createFromImageData(BinaryData::LAR_png, BinaryData::LAR_pngSize));
            modeMenu.addItem(4, "Right", true, false, juce::Drawable::createFromImageData(BinaryData::LAR2_png, BinaryData::LAR2_pngSize));
            modeMenu.addItem(5, "Stereo", true, false, juce::Drawable::createFromImageData(BinaryData::LR2_png, BinaryData::LR2_pngSize));
            modeMenu.addItem(6, "Side", true, false, juce::Drawable::createFromImageData(BinaryData::LR_png, BinaryData::LR_pngSize));
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
                        else if (result == 1)
                        {
                            juce::AlertWindow* aw = new juce::AlertWindow(
                                "Set position",
                                "Enter X and Y coordinates,please don't input zero or any number smaller then 0 :)",
                                juce::AlertWindow::QuestionIcon);

                            aw->setOpaque(false);
                            aw->setDropShadowEnabled(false);
                            aw->addTextEditor("X", "", "X");
                            aw->addTextEditor("Y", "", "Y");
                            aw->getTextEditor("X")->setInputRestrictions(4, "0123456789");
                            aw->getTextEditor("Y")->setInputRestrictions(4, "0123456789");
                            aw->addButton("OK", 1, juce::KeyPress(juce::KeyPress::returnKey));
                            aw->addButton("Cancel", 0, juce::KeyPress(juce::KeyPress::escapeKey));

                            aw->enterModalState(true, juce::ModalCallbackFunction::create(
                                [sp, aw](int result)
                                {
                                    if (result == 1)
                                    {
                                        int&& X = aw->getTextEditorContents("X").getIntValue();
                                        int&& Y = aw->getTextEditorContents("Y").getIntValue();
                                        if (X <= 0 || Y <= 0) return;
										sp->setBounds(X, Y, sp->getWidth(), sp->getHeight());
                                    }
                                    delete aw;
                                }
                            ), true);
                        }
                        else if (result == 2)
                        {
                            juce::AlertWindow* aw = new juce::AlertWindow(
                                "Resize Component",
                                "Enter Width and Height,please don't input zero or any number smaller then 100 :)",
                                juce::AlertWindow::QuestionIcon);

                            aw->setOpaque(false);
                            aw->setDropShadowEnabled(false);
                            aw->addTextEditor("Width", "", "Width");
                            aw->addTextEditor("Height", "", "Height");
                            aw->getTextEditor("Width")->setInputRestrictions(3, "0123456789");
                            aw->getTextEditor("Height")->setInputRestrictions(3, "0123456789");
                            aw->addButton("OK", 1, juce::KeyPress(juce::KeyPress::returnKey));
                            aw->addButton("Cancel", 0, juce::KeyPress(juce::KeyPress::escapeKey));

                            aw->enterModalState(true, juce::ModalCallbackFunction::create(
                                [sp, aw](int result)
                                {
                                    if (result == 1)
                                    {
                                        int&& Width = aw->getTextEditorContents("Width").getIntValue();
                                        int&& Height = aw->getTextEditorContents("Height").getIntValue();
                                        if (Width <= 100 || Height <= 100) return;
                                        sp->setBounds(sp->getX(), sp->getY(), Width, Height);
										sp->drawArea.setBounds(50, 65, Width - 65, Height - 100);
										sp->eqReferenceLines.setBounds(0, 50, Width, Height - 50);
										sp->componentHeader.setBounds(0, 0, Width, 50);
                                        sp->componentHeader.componentControl.setBounds(Width - 30, 20, 10, 10);
                                        sp->drawBounds.setBounds(0, 0, Width, Height);
                                    }
                                    delete aw;
                                }
                            ), true);
                        }
                        else if (result == 3)
                        {
                            sp->setAnalysisMode(AnalysisMode::Left);
							sp->eqReferenceLines.informationText = "CurrentMode: Left";
                            sp->eqReferenceLines.modeNum = 0;
							sp->eqReferenceLines.repaint();
                        }
                        else if (result == 4)
                        {
                            sp->setAnalysisMode(AnalysisMode::Right);
							sp->eqReferenceLines.informationText = "CurrentMode: Right";
                            sp->eqReferenceLines.modeNum = 1;
                            sp->eqReferenceLines.repaint();
                        }
                        else if (result == 5)
                        {
                            sp->setAnalysisMode(AnalysisMode::Stereo);
                            sp->eqReferenceLines.informationText = "CurrentMode: Stereo (L&R)";
                            sp->eqReferenceLines.modeNum = 3;
                            sp->eqReferenceLines.repaint();
                        }
                        else if (result == 6)
                        {
                            sp->setAnalysisMode(AnalysisMode::LR);
                            sp->eqReferenceLines.informationText = "CurrentMode: Side (L-R)";
                            sp->eqReferenceLines.modeNum = 4;
                            sp->eqReferenceLines.repaint();
                        }
                        else if (result == 7)
                        {
                            sp->setAnalysisMode(AnalysisMode::Interleaved);
                            sp->eqReferenceLines.informationText = "CurrentMode: Interleaved";
                            sp->eqReferenceLines.modeNum = 5;
                            sp->eqReferenceLines.repaint();
                        }
                    }
                });
        };

    spectrum->setBounds(x, y, 500, 150);
    spectrum->drawArea.setBounds(50, 65, 435, 50);
    spectrum->eqReferenceLines.setBounds(0, 50, 500, 100);
    spectrum->componentHeader.setBounds(0, 0, 500, 50);
    spectrum->componentHeader.themeConfigButton.setBounds(10, 10, 30, 30);
    spectrum->componentHeader.themeConfigButton.setTooltip("Theme Configuration");
	spectrum->componentHeader.headerFixedButton.setBounds(50, 10, 30, 30);
    spectrum->componentHeader.headerFixedButton.setTooltip("Fixed Header");
    spectrum->componentHeader.drawLinesButton.setBounds(90, 10, 30, 30);
    spectrum->componentHeader.drawLinesButton.onClick = [weakSpectrum]()
        {
            if (auto sp = weakSpectrum.lock())
            {
                sp->eqReferenceLines.setVisible(!sp->eqReferenceLines.isVisible());
            }
        };
}

void MainComponent::openSpectrumAnalyserMono(int x, int y)
{
    auto spectrum = ComponentManagement::getInstance().getSpectrumAnalyserMono();

    if (!spectrum->callbackId)
    {
        spectrum->callbackId = pushSampleIntoJuceAudioBuffer.add(
            [this, spectrum]()
            {
                spectrum->activityCheck();
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
    spectrum->drawBounds.setBounds(0, 0, 500, 150);

    std::weak_ptr<SpectrumAnalyserMono> weakSpectrum = spectrum;
    spectrum->componentHeader.componentControl.setCallBackFuntion([this, weakSpectrum]()
        {
            if (auto sp = weakSpectrum.lock())
            {
                pushSampleIntoJuceAudioBuffer.remove(sp->callbackId);
                pushSampleIntoJuceAudioBuffer.removeS(sp->callbackIdS);
                pushSampleIntoJuceAudioBuffer.removeM(sp->callbackIdM);
            }
            ComponentManagement::getInstance().resetSpectrumAnalyserMono();
        });

    spectrum->cb = [weakSpectrum]()
        {
            juce::PopupMenu menu;

            menu.addItem(1, "Position", true, false, juce::Drawable::createFromImageData(BinaryData::menuPosition_png, BinaryData::menuPosition_pngSize));
            menu.addItem(2, "ComponentSize", true, false, juce::Drawable::createFromImageData(BinaryData::menuComponentSize_png, BinaryData::menuComponentSize_pngSize));

            menu.showMenuAsync(
                juce::PopupMenu::Options(),
                [weakSpectrum](int result)
                {
                    if (auto sp = weakSpectrum.lock())
                    {
                        if (result == 0)
                        {
                        }
                        else if (result == 1)
                        {
                            juce::AlertWindow* aw = new juce::AlertWindow(
                                "Set position",
                                "Enter X and Y coordinates,please don't input zero or any number smaller then 0 :)",
                                juce::AlertWindow::QuestionIcon);

                            aw->setOpaque(false);
                            aw->setDropShadowEnabled(false);
                            aw->addTextEditor("X", "", "X");
                            aw->addTextEditor("Y", "", "Y");
                            aw->getTextEditor("X")->setInputRestrictions(4, "0123456789");
                            aw->getTextEditor("Y")->setInputRestrictions(4, "0123456789");
                            aw->addButton("OK", 1, juce::KeyPress(juce::KeyPress::returnKey));
                            aw->addButton("Cancel", 0, juce::KeyPress(juce::KeyPress::escapeKey));

                            aw->enterModalState(true, juce::ModalCallbackFunction::create(
                                [sp, aw](int result)
                                {
                                    if (result == 1)
                                    {
                                        int&& X = aw->getTextEditorContents("X").getIntValue();
                                        int&& Y = aw->getTextEditorContents("Y").getIntValue();
                                        if (X <= 0 || Y <= 0) return;
                                        sp->setBounds(X, Y, sp->getWidth(), sp->getHeight());
                                    }
                                    delete aw;
                                }
                            ), true);
                        }
                        else if (result == 2)
                        {
                            juce::AlertWindow* aw = new juce::AlertWindow(
                                "Resize Component",
                                "Enter Width and Height,please don't input zero or any number smaller then 100 :)",
                                juce::AlertWindow::QuestionIcon);

                            aw->setOpaque(false);
                            aw->setDropShadowEnabled(false);
                            aw->addTextEditor("Width", "", "Width");
                            aw->addTextEditor("Height", "", "Height");
                            aw->getTextEditor("Width")->setInputRestrictions(3, "0123456789");
                            aw->getTextEditor("Height")->setInputRestrictions(3, "0123456789");
                            aw->addButton("OK", 1, juce::KeyPress(juce::KeyPress::returnKey));
                            aw->addButton("Cancel", 0, juce::KeyPress(juce::KeyPress::escapeKey));

                            aw->enterModalState(true, juce::ModalCallbackFunction::create(
                                [sp, aw](int result)
                                {
                                    if (result == 1)
                                    {
                                        int&& Width = aw->getTextEditorContents("Width").getIntValue();
                                        int&& Height = aw->getTextEditorContents("Height").getIntValue();
                                        if (Width <= 100 || Height <= 100) return;
                                        sp->setBounds(sp->getX(), sp->getY(), Width, Height);
                                        sp->drawArea.setBounds(50, 65, Width - 65, Height - 100);
                                        sp->eqReferenceLines.setBounds(0, 50, Width, Height - 50);
                                        sp->componentHeader.setBounds(0, 0, Width, 50);
                                        sp->componentHeader.componentControl.setBounds(Width - 30, 20, 10, 10);
                                        sp->drawBounds.setBounds(0, 0, Width, Height);
                                    }
                                    delete aw;
                                }
                            ), true);
                        }
                    }
                });
        };

    spectrum->setBounds(x, y, 500, 150);
    spectrum->drawArea.setBounds(50, 65, 435, 50);
    spectrum->eqReferenceLines.setBounds(0, 50, 500, 100);
    spectrum->eqReferenceLines.informationText = "CurrentMode: Mono (L+R)";
    spectrum->componentHeader.setBounds(0, 0, 500, 50);
    spectrum->componentHeader.themeConfigButton.setBounds(10, 10, 30, 30);
    spectrum->componentHeader.themeConfigButton.setTooltip("Theme Configuration");
    spectrum->componentHeader.headerFixedButton.setBounds(50, 10, 30, 30);
    spectrum->componentHeader.headerFixedButton.setTooltip("Fixed Header");
    spectrum->componentHeader.drawLinesButton.setBounds(90, 10, 30, 30);
    spectrum->componentHeader.drawLinesButton.onClick = [weakSpectrum]()
        {
            if (auto sp = weakSpectrum.lock())
            {
                sp->eqReferenceLines.setVisible(!sp->eqReferenceLines.isVisible());
            }
        };
}

void MainComponent::openWaveformComponent(int x, int y)
{
    auto waveform = ComponentManagement::getInstance().getWaveformComponent();

    if (!waveform->callbackId)
    {
        waveform->callbackId = pushSampleIntoJuceAudioBuffer.add(
            [this, waveform]()
            {
                waveform->setWaveformData(
                    pushSampleIntoJuceAudioBuffer.getLocalAudioBufferReference(),
                    pushSampleIntoJuceAudioBuffer.getLocalAudioBufferRMSReference());
                waveform->repaint(waveform->getLocalBounds());
            });
    }

    addAndMakeVisible(waveform.get());

    waveform->setBounds(x, y, 1000, 150);
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
            menu.addItem(1, "Position", true, false, juce::Drawable::createFromImageData(BinaryData::menuPosition_png, BinaryData::menuPosition_pngSize));
            menu.addItem(2, "ComponentSize", true, false, juce::Drawable::createFromImageData(BinaryData::menuComponentSize_png, BinaryData::menuComponentSize_pngSize));
            menu.showMenuAsync(
                juce::PopupMenu::Options(),
                [weakVector](int result)
                {
                    if (auto vec = weakVector.lock())
                    {
                        if (result == 0)
                        {
                        }
                        else if (result == 1)
                        {
                            juce::AlertWindow* aw = new juce::AlertWindow(
                                "Set position",
                                "Enter X and Y coordinates,please don't input zero or any number smaller then 0 :)",
                                juce::AlertWindow::QuestionIcon);

                            aw->setOpaque(false);
                            aw->setDropShadowEnabled(false);
                            aw->addTextEditor("X", "", "X");
                            aw->addTextEditor("Y", "", "Y");
                            aw->getTextEditor("X")->setInputRestrictions(4, "0123456789");
                            aw->getTextEditor("Y")->setInputRestrictions(4, "0123456789");
                            aw->addButton("OK", 1, juce::KeyPress(juce::KeyPress::returnKey));
                            aw->addButton("Cancel", 0, juce::KeyPress(juce::KeyPress::escapeKey));

                            aw->enterModalState(true, juce::ModalCallbackFunction::create(
                                [vec, aw](int result)
                                {
                                    if (result == 1)
                                    {
                                        int&& X = aw->getTextEditorContents("X").getIntValue();
                                        int&& Y = aw->getTextEditorContents("Y").getIntValue();
                                        if (X <= 0 || Y <= 0) return;
                                        vec->setBounds(X, Y, vec->getWidth(), vec->getHeight());
                                    }
                                    delete aw;
                                }
                            ), true);
                        }
                        else if (result == 2)
                        {
                            juce::AlertWindow* aw = new juce::AlertWindow(
                                "Resize Component",
                                "Enter Width and Height,please don't input zero or any number smaller then 100 :)",
                                juce::AlertWindow::QuestionIcon);

                            aw->setOpaque(false);
                            aw->setDropShadowEnabled(false);
                            aw->addTextEditor("Width", "", "Width");
                            aw->addTextEditor("Height", "", "Height");
                            aw->getTextEditor("Width")->setInputRestrictions(3, "0123456789");
                            aw->getTextEditor("Height")->setInputRestrictions(3, "0123456789");
                            aw->addButton("OK", 1, juce::KeyPress(juce::KeyPress::returnKey));
                            aw->addButton("Cancel", 0, juce::KeyPress(juce::KeyPress::escapeKey));

                            aw->enterModalState(true, juce::ModalCallbackFunction::create(
                                [vec, aw](int result)
                                {
                                    if (result == 1)
                                    {
                                        int&& Width = aw->getTextEditorContents("Width").getIntValue();
                                        int&& Height = aw->getTextEditorContents("Height").getIntValue();
                                        if (Width <= 100 || Height <= 100) return;
                                        vec->setBounds(vec->getX(), vec->getY(), Width, Height);
                                        vec->componentHeader.setBounds(0, 0, Width, 50);
                                        vec->componentHeader.componentControl.setBounds(Width - 30, 20, 10, 10);
                                        vec->drawBounds.setBounds(0, 0, Width, Height);
                                        vec->oscilloscopeReferenceLines.setBounds(0, 0, Width, Height);
                                    }
                                    delete aw;
                                }
                            ), true);
                        }
                    }
                });
        };

    vector->setBounds(x, y, 300, 300);
    vector->componentHeader.setBounds(0, 0, 300, 50);
    vector->oscilloscopeReferenceLines.setBounds(0, 0, 300, 300);
    vector->componentHeader.themeConfigButton.setBounds(10, 10, 30, 30);
    vector->componentHeader.themeConfigButton.setTooltip("Theme Configuration");
    vector->componentHeader.headerFixedButton.setBounds(50, 10, 30, 30);
    vector->componentHeader.headerFixedButton.setTooltip("Fixed Header");
    vector->componentHeader.drawLinesButton.setBounds(90, 10, 30, 30);
    vector->componentHeader.drawLinesButton.onClick = [weakVector]()
        {
            if (auto sp = weakVector.lock())
            {
                sp->oscilloscopeReferenceLines.setVisible(!sp->oscilloscopeReferenceLines.isVisible());
            }
        };
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
            menu.addItem(1, "Position", true, false, juce::Drawable::createFromImageData(BinaryData::menuPosition_png, BinaryData::menuPosition_pngSize));
            menu.addItem(2, "ComponentSize", true, false, juce::Drawable::createFromImageData(BinaryData::menuComponentSize_png, BinaryData::menuComponentSize_pngSize));
            menu.showMenuAsync(
                juce::PopupMenu::Options(),
                [weakChart](int result)
                {
                    if (auto ch = weakChart.lock())
                    {
                        if (result == 0)
                        {
                        }
                        else if (result == 1)
                        {
                            juce::AlertWindow* aw = new juce::AlertWindow(
                                "Set position",
                                "Enter X and Y coordinates,please don't input zero or any number smaller then 0 :)",
                                juce::AlertWindow::QuestionIcon);

                            aw->setOpaque(false);
                            aw->setDropShadowEnabled(false);
                            aw->addTextEditor("X", "", "X");
                            aw->addTextEditor("Y", "", "Y");
                            aw->getTextEditor("X")->setInputRestrictions(4, "0123456789");
                            aw->getTextEditor("Y")->setInputRestrictions(4, "0123456789");
                            aw->addButton("OK", 1, juce::KeyPress(juce::KeyPress::returnKey));
                            aw->addButton("Cancel", 0, juce::KeyPress(juce::KeyPress::escapeKey));

                            aw->enterModalState(true, juce::ModalCallbackFunction::create(
                                [ch, aw](int result)
                                {
                                    if (result == 1)
                                    {
                                        int&& X = aw->getTextEditorContents("X").getIntValue();
                                        int&& Y = aw->getTextEditorContents("Y").getIntValue();
                                        if (X <= 0 || Y <= 0) return;
                                        ch->setBounds(X, Y, ch->getWidth(), ch->getHeight());
                                    }
                                    delete aw;
                                }
                            ), true);
                        }
                        else if (result == 2)
                        {
                            juce::AlertWindow* aw = new juce::AlertWindow(
                                "Resize Component",
                                "Enter Width and Height,please don't input zero or any number smaller then 100 :)",
                                juce::AlertWindow::QuestionIcon);

                            aw->setOpaque(false);
                            aw->setDropShadowEnabled(false);
                            aw->addTextEditor("Width", "", "Width");
                            aw->addTextEditor("Height", "", "Height");
                            aw->getTextEditor("Width")->setInputRestrictions(3, "0123456789");
                            aw->getTextEditor("Height")->setInputRestrictions(3, "0123456789");
                            aw->addButton("OK", 1, juce::KeyPress(juce::KeyPress::returnKey));
                            aw->addButton("Cancel", 0, juce::KeyPress(juce::KeyPress::escapeKey));

                            aw->enterModalState(true, juce::ModalCallbackFunction::create(
                                [ch, aw](int result)
                                {
                                    if (result == 1)
                                    {
                                        int&& Width = aw->getTextEditorContents("Width").getIntValue();
                                        int&& Height = aw->getTextEditorContents("Height").getIntValue();
                                        if (Width <= 100 || Height <= 100) return;
                                        ch->setBounds(ch->getX(), ch->getY(), Width, Height);
                                        ch->drawArea.setBounds(45, 50, Width - 45, Height - 70);
                                        ch->chartReferenceLine.setBounds(0, 50, Width, Height - 50);
                                        ch->componentHeader.setBounds(0, 0, Width, 50);
                                        ch->componentHeader.componentControl.setBounds(Width - 30, 20, 10, 10);
                                        ch->drawBounds.setBounds(0, 0, Width, Height);
                                    }
                                    delete aw;
                                }
                            ), true);
                        }
                    }
                });
        };

    chart->setBounds(x, y, 500, 150);
    chart->drawArea.setBounds(45, 50, 455, 80);
    chart->chartReferenceLine.setBounds(0, 50, 500, 100);
    chart->componentHeader.setBounds(0, 0, 500, 50);
    chart->componentHeader.themeConfigButton.setBounds(10, 10, 30, 30);
    chart->componentHeader.themeConfigButton.setTooltip("Theme Configuration");
    chart->componentHeader.headerFixedButton.setBounds(50, 10, 30, 30);
    chart->componentHeader.headerFixedButton.setTooltip("Fixed Header");
    chart->componentHeader.drawLinesButton.setBounds(90, 10, 30, 30);
    chart->componentHeader.drawLinesButton.onClick = [weakChart]()
        {
            if (auto sp = weakChart.lock())
            {
                sp->chartReferenceLine.setVisible(!sp->chartReferenceLine.isVisible());
            }
        };
}

void MainComponent::openRMSMeterComponent(int x, int y)
{
	auto rmsMeter = ComponentManagement::getInstance().getRMSMeterComponent();

    if (!rmsMeter->callbackId)
    {
        rmsMeter->callbackId = pushSampleIntoJuceAudioBuffer.add(
            [this, rmsMeter]()
            {
                rmsMeter->updateRMSValues(pushSampleIntoJuceAudioBuffer.getLeftLocalAudioBufferRMSReference(), pushSampleIntoJuceAudioBuffer.getRightLocalAudioBufferRMSReference(), pushSampleIntoJuceAudioBuffer.getLocalAudioBufferRMSReference());
            });
    }

    std::weak_ptr<RMSMeterComponent> weakRMSMeter = rmsMeter;
    rmsMeter->componentHeader.componentControl.setBounds(140, 20, 10, 10);
    rmsMeter->componentHeader.componentControl.setCallBackFuntion([this, weakRMSMeter]()
        {
            if (auto rms = weakRMSMeter.lock())
            {
                pushSampleIntoJuceAudioBuffer.remove(rms->callbackId);
            }
            ComponentManagement::getInstance().resetRMSMeterComponent();
        });

    rmsMeter->cb = [weakRMSMeter]()
        {
            juce::PopupMenu menu;
            menu.addItem(1, "Position", true, false, juce::Drawable::createFromImageData(BinaryData::menuPosition_png, BinaryData::menuPosition_pngSize));
            menu.addItem(2, "ComponentSize", true, false, juce::Drawable::createFromImageData(BinaryData::menuComponentSize_png, BinaryData::menuComponentSize_pngSize));
            menu.showMenuAsync(
                juce::PopupMenu::Options(),
                [weakRMSMeter](int result)
                {
                    if (auto rms = weakRMSMeter.lock())
                    {
                        if (result == 0)
                        {
                        }
                        else if (result == 1)
                        {
                            juce::AlertWindow* aw = new juce::AlertWindow(
                                "Set position",
                                "Enter X and Y coordinates,please don't input zero or any number smaller then 0 :)",
                                juce::AlertWindow::QuestionIcon);

                            aw->setOpaque(false);
                            aw->setDropShadowEnabled(false);
                            aw->addTextEditor("X", "", "X");
                            aw->addTextEditor("Y", "", "Y");
                            aw->getTextEditor("X")->setInputRestrictions(4, "0123456789");
                            aw->getTextEditor("Y")->setInputRestrictions(4, "0123456789");
                            aw->addButton("OK", 1, juce::KeyPress(juce::KeyPress::returnKey));
                            aw->addButton("Cancel", 0, juce::KeyPress(juce::KeyPress::escapeKey));

                            aw->enterModalState(true, juce::ModalCallbackFunction::create(
                                [rms, aw](int result)
                                {
                                    if (result == 1)
                                    {
                                        int&& X = aw->getTextEditorContents("X").getIntValue();
                                        int&& Y = aw->getTextEditorContents("Y").getIntValue();
                                        if (X <= 0 || Y <= 0) return;
                                        rms->setBounds(X, Y, rms->getWidth(), rms->getHeight());
                                    }
                                    delete aw;
                                }
                            ), true);
                        }
                        else if (result == 2)
                        {
                            juce::AlertWindow* aw = new juce::AlertWindow(
                                "Resize Component",
                                "Enter Width and Height,please don't input zero or any number smaller then 100 :)",
                                juce::AlertWindow::QuestionIcon);

                            aw->setOpaque(false);
                            aw->setDropShadowEnabled(false);
                            aw->addTextEditor("Width", "", "Width");
                            aw->addTextEditor("Height", "", "Height");
                            aw->getTextEditor("Width")->setInputRestrictions(3, "0123456789");
                            aw->getTextEditor("Height")->setInputRestrictions(3, "0123456789");
                            aw->addButton("OK", 1, juce::KeyPress(juce::KeyPress::returnKey));
                            aw->addButton("Cancel", 0, juce::KeyPress(juce::KeyPress::escapeKey));

                            aw->enterModalState(true, juce::ModalCallbackFunction::create(
                                [rms, aw](int result)
                                {
                                    if (result == 1)
                                    {
                                        int&& Width = aw->getTextEditorContents("Width").getIntValue();
                                        int&& Height = aw->getTextEditorContents("Height").getIntValue();
                                        if (Width <= 100 || Height <= 100) return;
                                        rms->setBounds(rms->getX(), rms->getY(), Width, Height);
                                        rms->ticksComponent.setBounds(0, 0, Width, Height);
                                        rms->componentHeader.setBounds(0, 0, Width, 50);
                                        rms->componentHeader.componentControl.setBounds(Width - 10, 20, 10, 10);
                                        rms->componentHeader.componentControl.setBounds(Width - 30, 20, 10, 10);
                                        rms->drawBounds.setBounds(0, 0, Width, Height);
                                    }
                                    delete aw;
                                }
                            ), true);
                        }
                    }
                });
        };
    addAndMakeVisible(rmsMeter.get());
    rmsMeter->setBounds(x, y, 150, 500);
    rmsMeter->drawBounds.setBounds(0, 0, 150, 500);
    rmsMeter->ticksComponent.setBounds(0, 0, 150, 500);
    rmsMeter->componentHeader.setBounds(0, 0, 150, 50);
    rmsMeter->componentHeader.themeConfigButton.setBounds(10, 10, 30, 30);
    rmsMeter->componentHeader.themeConfigButton.setTooltip("Theme Configuration");
    rmsMeter->componentHeader.headerFixedButton.setBounds(50, 10, 30, 30);
    rmsMeter->componentHeader.headerFixedButton.setTooltip("Fixed Header");
    rmsMeter->componentHeader.drawLinesButton.setBounds(90, 10, 30, 30);
    rmsMeter->componentHeader.drawLinesButton.onClick = [weakRMSMeter]()
        {
            if (auto rms = weakRMSMeter.lock())
            {
				rms->ticksComponent.setVisible(!rms->ticksComponent.isVisible());
            }
        };
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