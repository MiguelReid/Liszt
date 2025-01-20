/*
  ==============================================================================

    FilterControls.h
    Created: 17 Jan 2025 11:38:54am
    Author:  mikey

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "ToggleButton.h"
#include "Knob.h"

//==============================================================================
/*
*/
class FilterControls  : public juce::Component
{
public:
    FilterControls();
    ~FilterControls() override;

    void paint (juce::Graphics&) override;
    void resized() override;

private:
    // High-Pass Elements
    juce::Label highPassLabel;
    juce::ToggleButton highToggle;
    juce::Slider highCutoffKnob;
	juce::Label highCutoffLabel;
    juce::Slider highSlopeKnob;
	juce::Label highSlopeLabel;
    juce::Slider highEmphasisKnob;
	juce::Label highEmphasisLabel;

    // Low-Pass Elements
    juce::Label lowPassLabel;
    juce::ToggleButton lowToggle;
    juce::Slider lowCutoffKnob;
	juce::Label lowCutoffLabel;
    juce::Slider lowSlopeKnob;
	juce::Label lowSlopeLabel;
    juce::Slider lowEmphasisKnob;
	juce::Label lowEmphasisLabel;

    // Custom LookAndFeel
    std::unique_ptr<Knob> knobLookAndFeel;
    std::unique_ptr<ToggleButton> toggleButtonLookAndFeel;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (FilterControls)
};
