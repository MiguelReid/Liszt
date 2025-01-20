/*
  ==============================================================================

    ReverbControls.h
    Created: 12 Jan 2025 1:50:16pm
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
class ReverbControls  : public juce::Component
{
public:
    ReverbControls();
    ~ReverbControls() override;

    void paint (juce::Graphics&) override;
    void resized() override;

private:
    juce::Label reverbLabel;
    juce::Slider predelayKnob;
    juce::Slider decayKnob;
    juce::Slider dryWetKnob;
    juce::Slider diffusionKnob;

    juce::Label decayLabel;
    juce::Label predelayLabel;
    juce::Label dryWetLabel;
    juce::Label diffusionLabel;

    juce::ToggleButton reverbButton;

    std::unique_ptr<ToggleButton> toggleButtonLookAndFeel;
    std::unique_ptr<Knob> knobLookAndFeel;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ReverbControls)
};
