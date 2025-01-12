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
    auto labelHeight = 30;
    auto knobSize = 35;
    auto margin = 40;
    auto labelSize = 50;
    auto spaceForCenteredLabel = 3;

    // Position the label at the top
    reverbLabel.setBounds(bounds.removeFromTop(labelHeight));

    // Position the knobs in a 2x2 grid
    predelayKnob.setBounds(bounds.getX() + spaceForCenteredLabel, bounds.getY(), knobSize, knobSize);
    predelayLabel.setBounds(predelayKnob.getX() + spaceForCenteredLabel + (knobSize - labelSize) / 2, predelayKnob.getBottom() + 3, labelSize, 15);

    decayKnob.setBounds(bounds.getX() + knobSize + margin + spaceForCenteredLabel, bounds.getY(), knobSize, knobSize);
    decayLabel.setBounds(decayKnob.getX() + spaceForCenteredLabel + (knobSize - labelSize) / 2, decayKnob.getBottom() + 3, labelSize, 15);

    dryWetKnob.setBounds(bounds.getX() + spaceForCenteredLabel, bounds.getY() + knobSize + margin, knobSize, knobSize);
    dryWetLabel.setBounds(dryWetKnob.getX() + spaceForCenteredLabel + (knobSize - labelSize) / 2, dryWetKnob.getBottom() + 3, labelSize, 15);

    diffusionKnob.setBounds(bounds.getX() + spaceForCenteredLabel + knobSize + margin, bounds.getY() + knobSize + margin, knobSize, knobSize);
    diffusionLabel.setBounds(diffusionKnob.getX() + spaceForCenteredLabel + (knobSize - labelSize) / 2, diffusionKnob.getBottom() + 3, labelSize, 15);
}