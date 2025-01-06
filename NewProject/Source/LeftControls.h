/*
  ==============================================================================

    LeftControls.h
    Created: 5 Jan 2025 1:46:03pm
    Author:  mikey

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

//==============================================================================
/*
*/
class LeftControls  : public juce::Component
{
public:
    LeftControls();
    ~LeftControls() override;

    void paint (juce::Graphics&) override;
    void resized() override;

private:
    juce::ToggleButton arpeggiatorButton;
    juce::Slider pitchBendSlider;
};
