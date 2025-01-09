/*
  ==============================================================================

    Knob.cpp
    Created: 9 Jan 2025 5:39:46pm
    Author:  mikey

  ==============================================================================

*/

#include "Knob.h"

void Knob::drawRotarySlider(juce::Graphics& g, int x, int y, int width, int height,
    float sliderPosProportional, float rotaryStartAngle, float rotaryEndAngle, juce::Slider& slider)
{
    // Get bounds
    auto bounds = juce::Rectangle<int>(x, y, width, height).toFloat();
    auto radius = juce::jmin(bounds.getWidth(), bounds.getHeight()) / 2.0f;
    auto centerX = bounds.getCentreX();
    auto centerY = bounds.getCentreY();
    auto rx = centerX - radius;
    auto ry = centerY - radius;
    auto rw = radius * 2.0f;
    auto angle = rotaryStartAngle + sliderPosProportional * (rotaryEndAngle - rotaryStartAngle);

    // Draw background
    g.setColour(juce::Colours::darkgrey);
    g.fillEllipse(rx, ry, rw, rw);

    // Draw knob
    g.setColour(juce::Colours::lightblue);
    g.drawEllipse(rx, ry, rw, rw, 1.0f);

    // Draw pointer
    juce::Path p;
    auto pointerLength = radius * 0.6f;
    auto pointerThickness = 2.0f;
    p.addRectangle(-pointerThickness * 0.5f, -radius, pointerThickness, pointerLength);
    p.applyTransform(juce::AffineTransform::rotation(angle).translated(centerX, centerY));

    g.setColour(juce::Colours::white);
    g.fillPath(p);
}