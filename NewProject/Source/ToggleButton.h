/*
  ==============================================================================

    ToggleButton.h
    Created: 9 Jan 2025 5:52:29pm
    Author:  mikey

  ==============================================================================
*/

#pragma once

#include "JuceHeader.h"

class ToggleButton : public juce::LookAndFeel_V4
{
public:
    void drawToggleButton(juce::Graphics& g, juce::ToggleButton& button,
        bool shouldDrawButtonAsHighlighted, bool shouldDrawButtonAsDown) override;
};