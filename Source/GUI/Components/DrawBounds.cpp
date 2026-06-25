
#include "DrawBounds.h"

DrawBounds::DrawBounds()
{
	auto drawBoundsCat = CreateColoursConfiguration::getInstance().getCurrentValueTree()
		.getChildWithProperty("name", "ComponentBounds");
	boundsColour = juce::Colour(CreateColoursConfiguration::getInstance().colourHexToARGBInt(
		drawBoundsCat.getChildWithProperty("name", "ComponentBoundsColour").getProperty("hex").toString(), true));
	
	setInterceptsMouseClicks(false, false);
}

DrawBounds::~DrawBounds()
{
}

void DrawBounds::paint(juce::Graphics& g)
{
	g.setColour(boundsColour);
	g.drawRect(getLocalBounds());
}