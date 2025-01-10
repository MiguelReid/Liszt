/*
  ==============================================================================

    Knob.h
    Created: 9 Jan 2025 5:52:20pm
    Author:  mikey

  ==============================================================================
*/

#pragma once
#include <JuceHeader.h>

class Knob : public juce::LookAndFeel_V4
{
public:
    void drawRotarySlider(juce::Graphics& g, int x, int y, int width, int height,
        float sliderPosProportional, float rotaryStartAngle, float rotaryEndAngle, juce::Slider& slider) override;
};