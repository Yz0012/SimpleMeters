
#include "DrawBounds.h"

DrawBounds::DrawBounds()
{
}

DrawBounds::~DrawBounds()
{
}

void DrawBounds::paint(juce::Graphics& g)
{
	g.setColour(juce::Colour(CreateColoursConfiguration::getInstance().colourHexToARGBInt(
		CreateColoursConfiguration::getInstance().currentColourTheme.getChildWithProperty("name", "ComponentBoundsColour").getProperty("hex").toString(), true)));
	g.drawRect(getLocalBounds());
}