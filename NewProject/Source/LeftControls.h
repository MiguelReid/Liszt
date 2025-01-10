/*
  ==============================================================================

    LeftControls.h
    Created: 5 Jan 2025 1:46:03pm
    Author:  mikey

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "Knob.h"
#include "ToggleButton.h"

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
    juce::Slider gainSlider;

    // Labels
    juce::Label pitchBendLabel;
    juce::Label gainLabel;

    // Custom LookAndFeel
    std::unique_ptr<Knob> knobLookAndFeel;
    std::unique_ptr<ToggleButton> toggleButtonLookAndFeel;

};
