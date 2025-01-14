/*
  ==============================================================================

    ReverbControls.cpp
    Created: 12 Jan 2025 1:50:16pm
    Author:  mikey

  ==============================================================================
*/

#include "ReverbControls.h"

//==============================================================================
ReverbControls::ReverbControls()
{
    // Initialize and configure the label
    reverbLabel.setText("Reverb", juce::dontSendNotification);
    reverbLabel.setJustificationType(juce::Justification::centred);
    reverbLabel.setFont(juce::Font(18.0f, juce::Font::bold));
    reverbLabel.setColour(juce::Label::textColourId, juce::Colours::white);
    addAndMakeVisible(reverbLabel);

    // Initialize and configure the knobs
    predelayKnob.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    predelayKnob.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    addAndMakeVisible(predelayKnob);
    predelayLabel.setText("Predelay", juce::dontSendNotification);
    predelayLabel.setJustificationType(juce::Justification::centredTop);
    predelayLabel.setFont(12.0f);
    predelayLabel.setColour(juce::Label::textColourId, juce::Colours::white);
    addAndMakeVisible(predelayLabel);
    
    decayKnob.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    decayKnob.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    addAndMakeVisible(decayKnob);
    decayLabel.setText("Decay", juce::dontSendNotification);
    decayLabel.setJustificationType(juce::Justification::centredTop);
    decayLabel.setFont(12.0f);
    decayLabel.setColour(juce::Label::textColourId, juce::Colours::white);
    addAndMakeVisible(decayLabel);

    dryWetKnob.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    dryWetKnob.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    addAndMakeVisible(dryWetKnob);
    dryWetLabel.setText("Dry/Wet", juce::dontSendNotification);
    dryWetLabel.setJustificationType(juce::Justification::centredTop);
    dryWetLabel.setFont(12.0f);
    dryWetLabel.setColour(juce::Label::textColourId, juce::Colours::white);
    addAndMakeVisible(dryWetLabel);

    diffusionKnob.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    diffusionKnob.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    addAndMakeVisible(diffusionKnob);
    diffusionLabel.setText("Diffusion", juce::dontSendNotification);
    diffusionLabel.setJustificationType(juce::Justification::centredTop);
    diffusionLabel.setFont(12.0f);
    diffusionLabel.setColour(juce::Label::textColourId, juce::Colours::white);
    addAndMakeVisible(diffusionLabel);

    // ------------------- Look and feel --------------------------

    knobLookAndFeel = std::make_unique<Knob>();
    predelayKnob.setLookAndFeel(knobLookAndFeel.get());
    decayKnob.setLookAndFeel(knobLookAndFeel.get());
    dryWetKnob.setLookAndFeel(knobLookAndFeel.get());
    diffusionKnob.setLookAndFeel(knobLookAndFeel.get());
}

ReverbControls::~ReverbControls()
{
	predelayKnob.setLookAndFeel(nullptr);
	decayKnob.setLookAndFeel(nullptr);
	dryWetKnob.setLookAndFeel(nullptr);
	diffusionKnob.setLookAndFeel(nullptr);
}

void ReverbControls::paint (juce::Graphics& g)
{

}

void ReverbControls::resized()
{
    auto bounds = getLocalBounds();
    auto labelHeight = 20;
    auto knobSize = 55;
    auto margin = 27;
    auto labelSize = 50;
    auto extraSpace = 8;

    // Position the label at the top
    reverbLabel.setBounds(bounds.removeFromTop(labelHeight));

    // Position the knobs in a 2x2 grid
    predelayKnob.setBounds(bounds.getX(), bounds.getY() + extraSpace, knobSize, knobSize);
    predelayLabel.setBounds(predelayKnob.getX() + (knobSize - labelSize) / 2, predelayKnob.getBottom(), labelSize, 15);

    decayKnob.setBounds(bounds.getX() + knobSize + margin, bounds.getY() + extraSpace, knobSize, knobSize);
    decayLabel.setBounds(decayKnob.getX() + (knobSize - labelSize) / 2, decayKnob.getBottom(), labelSize, 15);

    dryWetKnob.setBounds(bounds.getX(), bounds.getY() + knobSize + margin + extraSpace, knobSize, knobSize);
    dryWetLabel.setBounds(dryWetKnob.getX() + (knobSize - labelSize) / 2, dryWetKnob.getBottom(), labelSize, 15);

    diffusionKnob.setBounds(bounds.getX() + knobSize + margin, bounds.getY() + knobSize + margin + extraSpace, knobSize, knobSize);
    diffusionLabel.setBounds(diffusionKnob.getX() + (knobSize - labelSize) / 2, diffusionKnob.getBottom(), labelSize, 15);
}