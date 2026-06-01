
#include "DrawBounds.h"

DrawBounds::DrawBounds()
{
	auto drawBoundsCat = CreateColoursConfiguration::getInstance().currentColourTheme
		.getChildWithProperty("name", "ComponentBounds");
	boundsColour = juce::Colour(CreateColoursConfiguration::getInstance().colourHexToARGBInt(
		drawBoundsCat.getChildWithProperty("name", "ComponentBoundsColour").getProperty("hex").toString(), true));
}

DrawBounds::~DrawBounds()
{
}

void DrawBounds::paint(juce::Graphics& g)
{
	g.setColour(boundsColour);
	g.drawRect(getLocalBounds());
}