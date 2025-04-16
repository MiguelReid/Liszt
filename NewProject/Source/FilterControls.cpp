/*
  ==============================================================================

    FilterControls.cpp
    Created: 17 Jan 2025 11:38:54am
    Author:  mikey

  ==============================================================================
*/

#include <JuceHeader.h>
#include "FilterControls.h"

//==============================================================================
FilterControls::FilterControls(juce::AudioProcessorValueTreeState& apvts) : apvts(apvts) {
    // Custom LookAndFeel
    knobLookAndFeel = std::make_unique<Knob>();

    // High-Pass Label
    highPassLabel.setText("High-Pass", juce::dontSendNotification);
    highPassLabel.setJustificationType(juce::Justification::centred);
    highPassLabel.setFont(juce::Font(18.0f, juce::Font::bold));
    highPassLabel.setColour(juce::Label::textColourId, juce::Colours::white);
    addAndMakeVisible(highPassLabel);

    // High-Pass Knob + Label
    highCutoffKnob.setSliderStyle(juce::Slider::Rotary);
    highCutoffKnob.setLookAndFeel(knobLookAndFeel.get());
    highCutoffKnob.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    addAndMakeVisible(highCutoffKnob);

    highCutoffLabel.setText("Cutoff", juce::dontSendNotification);
    highCutoffLabel.setJustificationType(juce::Justification::centredTop);
    highCutoffLabel.setFont(12.0f);
    highCutoffLabel.setColour(juce::Label::textColourId, juce::Colours::white);
    addAndMakeVisible(highCutoffLabel);

    // LOW PASS =================================================================

    // Low-Pass Label
    lowPassLabel.setText("Low-Pass", juce::dontSendNotification);
    lowPassLabel.setJustificationType(juce::Justification::centred);
    lowPassLabel.setFont(juce::Font(18.0f, juce::Font::bold));
    lowPassLabel.setColour(juce::Label::textColourId, juce::Colours::white);
    addAndMakeVisible(lowPassLabel);

    // Low-Pass Knob + Label
    lowCutoffKnob.setSliderStyle(juce::Slider::Rotary);
    lowCutoffKnob.setLookAndFeel(knobLookAndFeel.get());
    lowCutoffKnob.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    addAndMakeVisible(lowCutoffKnob);

    lowCutoffLabel.setText("Cutoff", juce::dontSendNotification);
    lowCutoffLabel.setJustificationType(juce::Justification::centredTop);
    lowCutoffLabel.setFont(12.0f);
    lowCutoffLabel.setColour(juce::Label::textColourId, juce::Colours::white);
    addAndMakeVisible(lowCutoffLabel);

    // AudioProcessorValueTreeState ===============================
    highCutoffAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        apvts, "HIGH_CUTOFF", highCutoffKnob);
    lowCutoffAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        apvts, "LOW_CUTOFF", lowCutoffKnob);
}

FilterControls::~FilterControls()
{
    highCutoffKnob.setLookAndFeel(nullptr);
    lowCutoffKnob.setLookAndFeel(nullptr);
}

void FilterControls::paint(juce::Graphics& g)
{
    // Empty paint method
}

void FilterControls::resized()
{
    auto area = getLocalBounds();
    int labelHeight = 20;
    int toggleSize = 20;
    int knobSize = 55;
    int knobLabelHeight = 15;
    int sectionSpacing = 5;

    // Calculate the height needed for each section
    int sectionHeight = labelHeight + knobSize + knobLabelHeight;

    // Center everything horizontally
    int centerX = area.getCentreX();

    // First Section: High-Pass
    // Place High-Pass Label centered above the knob
    highPassLabel.setBounds(
        centerX - 40, // Center the label
        area.getY(),
        80,
        labelHeight
    );

    // High Cutoff Knob - centered
    highCutoffKnob.setBounds(
        centerX - (knobSize / 2),
        highPassLabel.getBottom(),
        knobSize,
        knobSize
    );

    // High Cutoff Label - aligned with knob
    highCutoffLabel.setBounds(
        highCutoffKnob.getX(),
        highCutoffKnob.getBottom(),
        knobSize,
        knobLabelHeight
    );

    // Add spacing between sections
    int secondSectionY = highCutoffLabel.getBottom() + sectionSpacing;

    // Second Section: Low-Pass
    // Place Low-Pass Label centered above the knob
    lowPassLabel.setBounds(
        centerX - 40, // Center the label
        secondSectionY,
        80,
        labelHeight
    );

    // Low Cutoff Knob - centered
    lowCutoffKnob.setBounds(
        centerX - (knobSize / 2),
        lowPassLabel.getBottom(),
        knobSize,
        knobSize
    );

    // Low Cutoff Label - aligned with knob
    lowCutoffLabel.setBounds(
        lowCutoffKnob.getX(),
        lowCutoffKnob.getBottom(),
        knobSize,
        knobLabelHeight
    );
}
