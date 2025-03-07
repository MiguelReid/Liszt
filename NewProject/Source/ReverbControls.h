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
#include "PluginProcessor.h"

//==============================================================================
/*
*/
class ReverbControls : public juce::Component
{
public:
    ReverbControls(juce::AudioProcessorValueTreeState& apvts);
    ~ReverbControls() override;

    void paint(juce::Graphics&) override;
    void resized() override;

private:
    // APVTS Attachments
    std::unique_ptr <juce::AudioProcessorValueTreeState::SliderAttachment> predelayAttachment;
    std::unique_ptr <juce::AudioProcessorValueTreeState::SliderAttachment> decayAttachment;
    std::unique_ptr <juce::AudioProcessorValueTreeState::SliderAttachment> dryWetAttachment;
    std::unique_ptr <juce::AudioProcessorValueTreeState::SliderAttachment> diffusionAttachment;
    std::unique_ptr <juce::AudioProcessorValueTreeState::ButtonAttachment> reverbEnabledAttachment;

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

    juce::AudioProcessorValueTreeState& apvts;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ReverbControls)
};
