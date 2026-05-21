#include "VectorOscilloscopeComponent.h"

VectorOscilloscopes::VectorOscilloscopes(float x, float y)
{
	this->x = x; 
	this->y = y;
}

VectorOscilloscopes::~VectorOscilloscopes()
{
}

void VectorOscilloscopes::createStereoBufferAndPaint(float L , float R)
{
    if (bufferWritePosition >= stereoBuffer.getNumSamples()) return;
    auto* leftChannel = stereoBuffer.getWritePointer(0);
    auto* rightChannel = stereoBuffer.getWritePointer(1);
	leftChannel[bufferWritePosition] = L;
	rightChannel[bufferWritePosition] = R;
    bufferWritePosition = bufferWritePosition + 1;
}

void VectorOscilloscopes::paint(juce::Graphics& g)
{
    if (bufferWritePosition >= stereoBuffer.getNumSamples())
    {
		bufferWritePosition = 0;

        juce::Path p;
        bool firstPoint = true;

        for (int i = 0; i < stereoBuffer.getNumSamples(); ++i) {
            auto* leftSamples = stereoBuffer.getReadPointer(0);
            auto* rightSamples = stereoBuffer.getReadPointer(1);

            float mid = (leftSamples[i] + rightSamples[i]) * 0.5f;
            float side = (leftSamples[i] - rightSamples[i]) * 0.5f;

            float xMapping = (side + 1.0f) * x * 0.5f;
            float yMapping = (mid + 1.0f) * y * 0.5f;

            if (firstPoint) {
                p.startNewSubPath(xMapping, yMapping);
                firstPoint = false;
            }
            else {
                p.lineTo(xMapping, yMapping);
            }
        }

        g.setColour(juce::Colours::green);
        g.strokePath(p, juce::PathStrokeType(1.0f));
    }
}