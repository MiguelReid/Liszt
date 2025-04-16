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
    FilterControls(juce::AudioProcessorValueTreeState& apvts);
    ~FilterControls() override;

    void paint (juce::Graphics&) override;
    void resized() override;

private:
    // APVTS Attachments
    std::unique_ptr <juce::AudioProcessorValueTreeState::SliderAttachment> highCutoffAttachment;

    std::unique_ptr <juce::AudioProcessorValueTreeState::SliderAttachment> lowCutoffAttachment;

    // High-Pass Elements
    juce::Label highPassLabel;
    juce::Slider highCutoffKnob;
	juce::Label highCutoffLabel;

    // Low-Pass Elements
    juce::Label lowPassLabel;
    juce::Slider lowCutoffKnob;
	juce::Label lowCutoffLabel;

    // Custom LookAndFeel
    std::unique_ptr<Knob> knobLookAndFeel;

    juce::AudioProcessorValueTreeState& apvts;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (FilterControls)
};
