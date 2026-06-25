#include "RMSMeterComponent.h"

RMSMeterComponent::RMSMeterComponent()
{
    CreateColoursConfiguration& createColoursConfiguration = CreateColoursConfiguration::getInstance();

    rmsMeterCat = createColoursConfiguration.getCurrentValueTree()
        .getChildWithProperty("name", "RMSMeterComponent");
    
    buttomColor = juce::Colour(createColoursConfiguration.colourHexToARGBInt(
        rmsMeterCat.getChildWithProperty("name", "Bottom").getProperty("hex").toString(), false));
    topColor = juce::Colour(createColoursConfiguration.colourHexToARGBInt(
        rmsMeterCat.getChildWithProperty("name", "Top").getProperty("hex").toString(), false));

    addAndMakeVisible(&drawBounds);
    addAndMakeVisible(&componentHeader);
    addAndMakeVisible(&ticksComponent);

    startTimerHz(30);

	rmsMeterCat.addListener(this);
}

RMSMeterComponent::~RMSMeterComponent()
{
    stopTimer();
    rmsMeterCat.removeListener(this);
}

void RMSMeterComponent::paint(juce::Graphics& g)
{
    auto bounds = getLocalBounds().toFloat();
    float meterWidth = bounds.getWidth() - leftMargin;
    float meterHeight = bounds.getHeight() - bottomMargin;

    float barWidth = meterWidth * 0.22f;
    float gap = (meterWidth - 3.0f * barWidth) / 4.0f;
    float zeroDbY = amplitudeToY(1.0f, meterHeight);

    for (int i = 0; i < 3; ++i)
    {
        float xOffset = leftMargin + gap + i * (barWidth + gap);

        float currentY = amplitudeToY(rmsValues[i], meterHeight);
        float safeTopY = std::max(currentY, zeroDbY);
        if (safeTopY < meterHeight)
        {
            juce::Rectangle<float> safeRect(xOffset, safeTopY, barWidth, meterHeight - safeTopY);
            juce::ColourGradient gradient(buttomColor, 0.0f, meterHeight, topColor, 0.0f, zeroDbY, false);
            g.setGradientFill(gradient);
            g.fillRect(safeRect);
        }
        if (currentY < zeroDbY)
        {
            juce::Rectangle<float> redRect(xOffset, currentY, barWidth, zeroDbY - currentY);
            g.setColour(juce::Colours::red);
            g.fillRect(redRect);
        }

        float peakY = amplitudeToY(peakValues[i], meterHeight);
        if (peakY < meterHeight&& peakY >= 0.0f)
        {
            bool isOverCeiling = peakValues[i] > 1.0f;
            g.setColour(isOverCeiling ? juce::Colours::red : juce::Colours::white);
            g.drawHorizontalLine(static_cast<int>(peakY), xOffset, xOffset + barWidth);

            float peakDb = juce::Decibels::gainToDecibels(peakValues[i], -60.0f);

            juce::String peakText;
            if (peakDb <= -60.0f) {
                peakText = "-inf";
            }
            else {
                peakText = (peakDb > 0.0f ? "+" : "") + juce::String(peakDb, 1);
            }

            g.setFont(10.0f);
            g.setColour(isOverCeiling ? juce::Colours::red : juce::Colours::white.withAlpha(0.9f));

            float textY = (peakY > 14.0f) ? (peakY - 14.0f) : 2.0f;

            g.drawText(peakText,
                xOffset - 5.0f,
                textY,
                barWidth + 10.0f,
                12.0f,
                juce::Justification::centred);
        }
    }
}

void RMSMeterComponent::resized() {}

void RMSMeterComponent::updateRMSValues(float left, float right, float total)
{
    float newValues[3] = { left, right, total };
    auto currentTime = juce::Time::getApproximateMillisecondCounter();

    for (int i = 0; i < 3; ++i)
    {
        rmsValues[i] = juce::jlimit(0.0f, 4.0f, newValues[i]);

        if (rmsValues[i] > peakValues[i] || currentTime >= peakTimers[i])
        {
            peakValues[i] = rmsValues[i];
            peakTimers[i] = currentTime + 1000;
        }
    }

    repaint();
}

void RMSMeterComponent::timerCallback()
{
    auto currentTime = juce::Time::getApproximateMillisecondCounter();
    bool needsRepaint = false;

    for (int i = 0; i < 3; ++i)
    {
        if (currentTime >= peakTimers[i])
        {
            peakValues[i] = rmsValues[i];
            peakTimers[i] = currentTime + 1000;
            needsRepaint = true;
        }
    }

    if (needsRepaint) repaint();
}

float RMSMeterComponent::amplitudeToY(float amplitude, float availableHeight) const
{
    float db = juce::Decibels::gainToDecibels(std::max(0.00001f, amplitude), -60.0f);
    return juce::jmap(db, -60.0f, 12.0f, availableHeight, 0.0f);
}

void RMSMeterComponent::mouseDown(const juce::MouseEvent& event)
{
    if (event.mods.isPopupMenu())
    {
        if (cb == nullptr) return;
        cb();
    }
}

void RMSMeterComponent::mouseEnter(const juce::MouseEvent& event)
{
    drawBounds.setVisible(true);
    componentHeader.setVisible(true);
}

void RMSMeterComponent::mouseExit(const juce::MouseEvent&)
{
    if (!componentHeader.isMouseOver() && !componentHeader.headerFixedButton.getHeaderFixed())
    {
        drawBounds.setVisible(false);
        componentHeader.setVisible(false);
    }
}

void RMSMeterComponent::valueTreePropertyChanged(juce::ValueTree& tree, const juce::Identifier& property)
{
    if (property == juce::Identifier("hex"))
    {
        this->buttomColor = juce::Colour(CreateColoursConfiguration::getInstance().colourHexToARGBInt(
            rmsMeterCat.getChildWithProperty("name", "Bottom").getProperty("hex").toString(), false));
        this->topColor = juce::Colour(CreateColoursConfiguration::getInstance().colourHexToARGBInt(
            rmsMeterCat.getChildWithProperty("name", "Top").getProperty("hex").toString(), false));
        repaint();
    }
}