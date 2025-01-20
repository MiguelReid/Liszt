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
    OscillatorControls();
    ~OscillatorControls() override;

    void paint (juce::Graphics&) override;
    void resized() override;

private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(OscillatorControls)
    juce::Label oscillatorLabel;
    juce::Slider rangeKnob1;
	juce::Slider rangeKnob2;
	juce::Slider shapeKnob1;
	juce::Slider shapeKnob2;

	juce::Label oscLabel1;
    juce::Label oscLabel2;
	juce::Label rangeLabel1;
	juce::Label rangeLabel2;
	juce::Label shapeLabel1;
	juce::Label shapeLabel2;

    juce::ToggleButton osc1Button;
    juce::ToggleButton osc2Button;

    std::unique_ptr<Knob> knobLookAndFeel;
	std::unique_ptr<ToggleButton> toggleButtonLookAndFeel;
};
