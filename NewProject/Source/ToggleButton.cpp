/*
  ==============================================================================

    ToggleButton.cpp
    Created: 9 Jan 2025 5:39:46pm
    Author:  mikey

  ==============================================================================

*/

#include "ToggleButton.h"

void ToggleButton::drawToggleButton(juce::Graphics& g, juce::ToggleButton& button,
    bool shouldDrawButtonAsHighlighted, bool shouldDrawButtonAsDown)
{
    // Get button bounds
    auto bounds = button.getLocalBounds().reduced(4);
    auto isToggled = button.getToggleState();

    // Draw background
    g.setColour(juce::Colours::darkgrey);
    g.fillRoundedRectangle(bounds.toFloat(), 4.0f);

    // Draw toggle switch base
    auto switchWidth = bounds.getHeight() * 0.4f;
    auto switchHeight = bounds.getHeight() * 0.6f;

    auto switchX = isToggled ? bounds.getRight() - switchWidth - 6
        : bounds.getX() + 6;

    auto switchY = bounds.getY() + (bounds.getHeight() - switchHeight) / 2;

    g.setColour(isToggled ? juce::Colours::lightblue : juce::Colours::darkblue);
    g.fillRect(switchX, switchY, switchWidth, switchHeight);

    // Draw "ON" and "OFF" text
    g.setFont(12.0f);
    g.setColour(juce::Colours::white);
    g.drawText("ON", bounds.withWidth(bounds.getWidth() / 2), juce::Justification::centredRight);
    g.drawText("OFF", bounds.withWidth(bounds.getWidth() / 2), juce::Justification::centredLeft);
}
