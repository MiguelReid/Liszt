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
    // Define the diameter of the circle
    const float circleDiameter = 10.0f;  // Adjust as needed for size

    // Get the bounds of the button
    auto bounds = button.getLocalBounds().toFloat();

    // Calculate the center position for the circle
    float centerX = bounds.getCentreX();
    float centerY = bounds.getY() + circleDiameter / 2 + 5.0f;  // Adjust vertical position if needed

    // Define the circle area
    juce::Rectangle<float> circleArea(centerX - circleDiameter / 2,
        centerY - circleDiameter / 2,
        circleDiameter,
        circleDiameter);

    // Determine the color based on the toggle state
    juce::Colour circleColour = button.getToggleState() ? juce::Colours::red : juce::Colours::black;

    // Draw the circle
    g.setColour(circleColour);
    g.fillEllipse(circleArea);

    // Optional: Draw a border around the circle
    g.setColour(juce::Colours::grey);
    g.drawEllipse(circleArea, 1.0f);

    // Draw the button text below the circle
    juce::Font font(12.0f);
    g.setFont(font);
    g.setColour(juce::Colours::white);

    float textWidth = bounds.getWidth();
    float textHeight = 20.0f;  // Height of the text area
    float textY = circleArea.getBottom() + 5.0f;  // Space between circle and text

    juce::Rectangle<float> textArea(bounds.getX(), textY, textWidth, textHeight);

    g.drawFittedText(button.getButtonText(), textArea.toNearestInt(), juce::Justification::centredTop, 1);
}

