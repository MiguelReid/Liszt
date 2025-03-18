/*
  ==============================================================================

    OscillatorControls.h
    Created: 13 Jan 2025 12:48:25pm
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
class OscillatorControls  : public juce::Component
{
public:
    OscillatorControls(juce::AudioProcessorValueTreeState& apvts);
    ~OscillatorControls() override;

    void paint (juce::Graphics&) override;
    void resized() override;

private:
	// APVTS Attachments
	std::unique_ptr <juce::AudioProcessorValueTreeState::SliderAttachment> depthAttachment1;
	std::unique_ptr <juce::AudioProcessorValueTreeState::SliderAttachment> depthAttachment2;
	std::unique_ptr <juce::AudioProcessorValueTreeState::SliderAttachment> shapeAttachment1;
	std::unique_ptr <juce::AudioProcessorValueTreeState::SliderAttachment> shapeAttachment2;
	std::unique_ptr <juce::AudioProcessorValueTreeState::ButtonAttachment> oscAttachment1;
	std::unique_ptr <juce::AudioProcessorValueTreeState::ButtonAttachment> oscAttachment2;

	// Elements
    juce::Label oscillatorLabel;
    juce::Slider depthKnob1;
	juce::Slider depthKnob2;
	juce::Slider shapeKnob1;
	juce::Slider shapeKnob2;

	juce::Label oscLabel1;
    juce::Label oscLabel2;
	juce::Label depthLabel1;
	juce::Label depthLabel2;
	juce::Label shapeLabel1;
	juce::Label shapeLabel2;

    juce::ToggleButton osc1Button;
    juce::ToggleButton osc2Button;

	// Custom LookAndFeel
    std::unique_ptr<Knob> knobLookAndFeel;
	std::unique_ptr<ToggleButton> toggleButtonLookAndFeel;

	// APVTS
    juce::AudioProcessorValueTreeState& apvts;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(OscillatorControls)
};
