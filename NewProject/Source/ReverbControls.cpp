/*
  ==============================================================================

    ReverbControls.cpp
    Created: 12 Jan 2025 1:50:16pm
    Author:  mikey

  ==============================================================================
*/

#include "ReverbControls.h"

//==============================================================================
ReverbControls::ReverbControls(juce::AudioProcessorValueTreeState& apvts): apvts(apvts) {
    // Initialize and configure the label
    reverbLabel.setText("Reverb", juce::dontSendNotification);
    reverbLabel.setJustificationType(juce::Justification::centred);
    reverbLabel.setFont(juce::Font(18.0f, juce::Font::bold));
    reverbLabel.setColour(juce::Label::textColourId, juce::Colours::white);
    addAndMakeVisible(reverbLabel);

    // Initialize and configure the knobs
    predelayKnob.setSliderStyle(juce::Slider::Rotary);
    predelayKnob.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    addAndMakeVisible(predelayKnob);
    predelayLabel.setText("Predelay", juce::dontSendNotification);
    predelayLabel.setJustificationType(juce::Justification::centredTop);
    predelayLabel.setFont(12.0f);
    predelayLabel.setColour(juce::Label::textColourId, juce::Colours::white);
    addAndMakeVisible(predelayLabel);
    
    decayKnob.setSliderStyle(juce::Slider::Rotary);
    decayKnob.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    addAndMakeVisible(decayKnob);
    decayLabel.setText("Decay", juce::dontSendNotification);
    decayLabel.setJustificationType(juce::Justification::centredTop);
    decayLabel.setFont(12.0f);
    decayLabel.setColour(juce::Label::textColourId, juce::Colours::white);
    addAndMakeVisible(decayLabel);

    dryWetKnob.setSliderStyle(juce::Slider::Rotary);
    dryWetKnob.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    addAndMakeVisible(dryWetKnob);
    dryWetLabel.setText("Dry/Wet", juce::dontSendNotification);
    dryWetLabel.setJustificationType(juce::Justification::centredTop);
    dryWetLabel.setFont(12.0f);
    dryWetLabel.setColour(juce::Label::textColourId, juce::Colours::white);
    addAndMakeVisible(dryWetLabel);

    diffusionKnob.setSliderStyle(juce::Slider::Rotary);
    diffusionKnob.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    addAndMakeVisible(diffusionKnob);
    diffusionLabel.setText("Diffusion", juce::dontSendNotification);
    diffusionLabel.setJustificationType(juce::Justification::centredTop);
    diffusionLabel.setFont(12.0f);
    diffusionLabel.setColour(juce::Label::textColourId, juce::Colours::white);
    addAndMakeVisible(diffusionLabel);

    // High-Pass Knob + Label
    highCutoffKnob.setSliderStyle(juce::Slider::Rotary);
    highCutoffKnob.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    addAndMakeVisible(highCutoffKnob);
    highCutoffLabel.setText("HP Cutoff", juce::dontSendNotification);
    highCutoffLabel.setJustificationType(juce::Justification::centredTop);
    highCutoffLabel.setFont(12.0f);
    highCutoffLabel.setColour(juce::Label::textColourId, juce::Colours::white);
    addAndMakeVisible(highCutoffLabel);

    // Low-Pass Knob + Label
    lowCutoffKnob.setSliderStyle(juce::Slider::Rotary);
    lowCutoffKnob.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    addAndMakeVisible(lowCutoffKnob);

    lowCutoffLabel.setText("LP Cutoff", juce::dontSendNotification);
    lowCutoffLabel.setJustificationType(juce::Justification::centredTop);
    lowCutoffLabel.setFont(12.0f);
    lowCutoffLabel.setColour(juce::Label::textColourId, juce::Colours::white);
    addAndMakeVisible(lowCutoffLabel);

	addAndMakeVisible(reverbButton);

    // =================== Look and feel ==========================

    knobLookAndFeel = std::make_unique<Knob>();
	toggleButtonLookAndFeel = std::make_unique<ToggleButton>();

    predelayKnob.setLookAndFeel(knobLookAndFeel.get());
    decayKnob.setLookAndFeel(knobLookAndFeel.get());
    dryWetKnob.setLookAndFeel(knobLookAndFeel.get());
    diffusionKnob.setLookAndFeel(knobLookAndFeel.get());
    lowCutoffKnob.setLookAndFeel(knobLookAndFeel.get());
    highCutoffKnob.setLookAndFeel(knobLookAndFeel.get());
    reverbButton.setLookAndFeel(toggleButtonLookAndFeel.get());

	// AudioProcessorValueTreeState ===============================
	predelayAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        apvts, "PREDELAY", predelayKnob);

    decayAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        apvts, "DECAY", decayKnob);

    dryWetAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        apvts, "DRYWET", dryWetKnob);

    diffusionAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        apvts, "DIFFUSION", diffusionKnob);

    highCutoffAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        apvts, "HIGH_CUTOFF", highCutoffKnob);

    lowCutoffAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        apvts, "LOW_CUTOFF", lowCutoffKnob);

    reverbEnabledAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(
        apvts, "REVERB_ENABLED", reverbButton);
}


ReverbControls::~ReverbControls()
{
	predelayKnob.setLookAndFeel(nullptr);
	decayKnob.setLookAndFeel(nullptr);
	dryWetKnob.setLookAndFeel(nullptr);
	diffusionKnob.setLookAndFeel(nullptr);
	reverbButton.setLookAndFeel(nullptr);
    highCutoffKnob.setLookAndFeel(nullptr);
    lowCutoffKnob.setLookAndFeel(nullptr);
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
    auto buttonWidth = 20;
    auto buttonHeight = 20;

    // Calculate total width for the label and button
    auto totalWidth = reverbLabel.getFont().getStringWidth(reverbLabel.getText()) + buttonWidth + margin;
    auto reverbLabelWidth = reverbLabel.getFont().getStringWidth(reverbLabel.getText());

    // Position the label at the top
    reverbLabel.setBounds((bounds.getWidth() - reverbLabelWidth - buttonWidth) / 2, 0, reverbLabelWidth, labelHeight);
    reverbButton.setBounds(reverbLabel.getRight(), reverbLabel.getY() + (labelHeight - buttonHeight) / 2, buttonWidth, buttonHeight);

    // Adjust bounds to leave space for the label and button
    bounds.removeFromTop(labelHeight + extraSpace);

    // Position the knobs in a 3x2 grid (3 columns, 2 rows)

    // Row 1, Column 1
    predelayKnob.setBounds(bounds.getX(), bounds.getY(), knobSize, knobSize);
    predelayLabel.setBounds(predelayKnob.getX() + (knobSize - labelSize) / 2, predelayKnob.getBottom(), labelSize, 15);

    // Row 1, Column 2
    decayKnob.setBounds(bounds.getX() + knobSize + margin, bounds.getY(), knobSize, knobSize);
    decayLabel.setBounds(decayKnob.getX() + (knobSize - labelSize) / 2, decayKnob.getBottom(), labelSize, 15);

    // Row 1, Column 3 (High Cutoff)
    highCutoffKnob.setBounds(bounds.getX() + 2 * (knobSize + margin), bounds.getY(), knobSize, knobSize);
    highCutoffLabel.setBounds(highCutoffKnob.getX(), highCutoffKnob.getBottom(), knobSize, 15);

    // Row 2, Column 1
    dryWetKnob.setBounds(bounds.getX(), bounds.getY() + knobSize + margin, knobSize, knobSize);
    dryWetLabel.setBounds(dryWetKnob.getX() + (knobSize - labelSize) / 2, dryWetKnob.getBottom(), labelSize, 15);

    // Row 2, Column 2
    diffusionKnob.setBounds(bounds.getX() + knobSize + margin, bounds.getY() + knobSize + margin, knobSize, knobSize);
    diffusionLabel.setBounds(diffusionKnob.getX() + (knobSize - labelSize) / 2, diffusionKnob.getBottom(), labelSize, 15);

    // Row 2, Column 3 (Low Cutoff)
    lowCutoffKnob.setBounds(bounds.getX() + 2 * (knobSize + margin), bounds.getY() + knobSize + margin, knobSize, knobSize);
    lowCutoffLabel.setBounds(lowCutoffKnob.getX(), lowCutoffKnob.getBottom(), knobSize, 15);
}
