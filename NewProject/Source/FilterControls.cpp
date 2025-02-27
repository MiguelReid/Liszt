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
    toggleButtonLookAndFeel = std::make_unique<ToggleButton>();
    knobLookAndFeel = std::make_unique<Knob>();

    // High-Pass Label
    highPassLabel.setText("High-Pass", juce::dontSendNotification);
    highPassLabel.setJustificationType(juce::Justification::centred);
    highPassLabel.setFont(juce::Font(18.0f, juce::Font::bold));
    highPassLabel.setColour(juce::Label::textColourId, juce::Colours::white);
    addAndMakeVisible(highPassLabel);

    // High-Pass ToggleButton
    highToggle.setLookAndFeel(toggleButtonLookAndFeel.get());
    addAndMakeVisible(highToggle);

    // High-Pass Knobs + Labels
    highCutoffKnob.setSliderStyle(juce::Slider::Rotary);
    highCutoffKnob.setLookAndFeel(knobLookAndFeel.get());
    highCutoffKnob.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    addAndMakeVisible(highCutoffKnob);
    highCutoffLabel.setText("Cutoff", juce::dontSendNotification);
    highCutoffLabel.setJustificationType(juce::Justification::centredTop);
    highCutoffLabel.setFont(12.0f);
    highCutoffLabel.setColour(juce::Label::textColourId, juce::Colours::white);
    addAndMakeVisible(highCutoffLabel);

    highSlopeKnob.setSliderStyle(juce::Slider::Rotary);
    highSlopeKnob.setLookAndFeel(knobLookAndFeel.get());
	highSlopeKnob.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    addAndMakeVisible(highSlopeKnob);
    highSlopeLabel.setText("Slope", juce::dontSendNotification);
    highSlopeLabel.setJustificationType(juce::Justification::centredTop);
    highSlopeLabel.setFont(12.0f);
    highSlopeLabel.setColour(juce::Label::textColourId, juce::Colours::white);
    addAndMakeVisible(highSlopeLabel);

    highEmphasisKnob.setSliderStyle(juce::Slider::Rotary);
    highEmphasisKnob.setLookAndFeel(knobLookAndFeel.get());
	highEmphasisKnob.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    addAndMakeVisible(highEmphasisKnob);
    highEmphasisLabel.setText("Emphasis", juce::dontSendNotification);
    highEmphasisLabel.setJustificationType(juce::Justification::centredTop);
    highEmphasisLabel.setFont(12.0f);
    highEmphasisLabel.setColour(juce::Label::textColourId, juce::Colours::white);
    addAndMakeVisible(highEmphasisLabel);

    // LOW PASS =================================================================

    // Low-Pass Label
    lowPassLabel.setText("Low-Pass", juce::dontSendNotification);
    lowPassLabel.setJustificationType(juce::Justification::centred);
    lowPassLabel.setFont(juce::Font(18.0f, juce::Font::bold));
    lowPassLabel.setColour(juce::Label::textColourId, juce::Colours::white);
    addAndMakeVisible(lowPassLabel);

    // Low-Pass ToggleButton
    lowToggle.setLookAndFeel(toggleButtonLookAndFeel.get());
    addAndMakeVisible(lowToggle);

    // Low-Pass Knobs + Labels
    lowCutoffKnob.setSliderStyle(juce::Slider::Rotary);
    lowCutoffKnob.setLookAndFeel(knobLookAndFeel.get());
	lowCutoffKnob.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    addAndMakeVisible(lowCutoffKnob);
    lowCutoffLabel.setText("Cutoff", juce::dontSendNotification);
    lowCutoffLabel.setJustificationType(juce::Justification::centredTop);
    lowCutoffLabel.setFont(12.0f);
    lowCutoffLabel.setColour(juce::Label::textColourId, juce::Colours::white);
    addAndMakeVisible(lowCutoffLabel);

    lowSlopeKnob.setSliderStyle(juce::Slider::Rotary);
    lowSlopeKnob.setLookAndFeel(knobLookAndFeel.get());
	lowSlopeKnob.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    addAndMakeVisible(lowSlopeKnob);
    lowSlopeLabel.setText("Slope", juce::dontSendNotification);
    lowSlopeLabel.setJustificationType(juce::Justification::centredTop);
    lowSlopeLabel.setFont(12.0f);
    lowSlopeLabel.setColour(juce::Label::textColourId, juce::Colours::white);
    addAndMakeVisible(lowSlopeLabel);

    lowEmphasisKnob.setSliderStyle(juce::Slider::Rotary);
    lowEmphasisKnob.setLookAndFeel(knobLookAndFeel.get());
	lowEmphasisKnob.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    addAndMakeVisible(lowEmphasisKnob);
    lowEmphasisLabel.setText("Emphasis", juce::dontSendNotification);
    lowEmphasisLabel.setJustificationType(juce::Justification::centredTop);
    lowEmphasisLabel.setFont(12.0f);
    lowEmphasisLabel.setColour(juce::Label::textColourId, juce::Colours::white);
    addAndMakeVisible(lowEmphasisLabel);

	// AudioProcessorValueTreeState ===============================
	highCutoffAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
		apvts, "HIGH_CUTOFF", highCutoffKnob);
	highSlopeAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
		apvts, "HIGH_SLOPE", highSlopeKnob);
	highEmphasisAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
		apvts, "HIGH_EMPHASIS", highEmphasisKnob);
	lowCutoffAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
		apvts, "LOW_CUTOFF", lowCutoffKnob);
	lowSlopeAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
		apvts, "LOW_SLOPE", lowSlopeKnob);
	lowEmphasisAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
		apvts, "LOW_EMPHASIS", lowEmphasisKnob);
	highToggleAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(
		apvts, "HIGH_ENABLED", highToggle);
	lowToggleAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(
		apvts, "LOW_ENABLED", lowToggle);
}

FilterControls::~FilterControls()
{
    // Reset LookAndFeel
    highToggle.setLookAndFeel(nullptr);
    lowToggle.setLookAndFeel(nullptr);

    highCutoffKnob.setLookAndFeel(nullptr);
    highSlopeKnob.setLookAndFeel(nullptr);
    highEmphasisKnob.setLookAndFeel(nullptr);

    lowCutoffKnob.setLookAndFeel(nullptr);
    lowSlopeKnob.setLookAndFeel(nullptr);
    lowEmphasisKnob.setLookAndFeel(nullptr);
}

void FilterControls::paint (juce::Graphics& g)
{

}

void FilterControls::resized()
{
    auto area = getLocalBounds();
    int labelHeight = 20;
    int toggleSize = 20;
    int knobSize = 55;
    int knobLabelHeight = 15; // Height for the knob labels
    int spacing = 5; // Spacing between components
    int sectionSpacing = 5; // Additional spacing between High-Pass and Low-Pass sections

    // Total height allocated to each section (High-Pass and Low-Pass)
    int sectionHeight = labelHeight + knobSize + knobLabelHeight + spacing * 2;

    // First Section: High-Pass
    auto highPassArea = area.removeFromTop(sectionHeight);

    // Place High-Pass Label
    highPassLabel.setBounds(
        highPassArea.getCentreX() - 53, // Center the label horizontally
        highPassArea.getY(),
        80,
        labelHeight
    );

    // Place High-Pass ToggleButton
    highToggle.setBounds(
        highPassLabel.getRight() + spacing,
        highPassArea.getY(),
        toggleSize,
        toggleSize
    );

    // Place High-Pass Knobs and their Labels
    int totalKnobWidth = (3 * knobSize) + (2 * spacing);
    int knobsStartX = highPassArea.getCentreX() - (totalKnobWidth / 2);
    int knobsY = highPassLabel.getBottom() + spacing;

    // High Cutoff Knob
    highCutoffKnob.setBounds(knobsStartX, knobsY, knobSize, knobSize);
    highCutoffLabel.setBounds(
        highCutoffKnob.getX(),
        highCutoffKnob.getBottom(),
        knobSize,
        knobLabelHeight
    );

    // High Slope Knob
    highSlopeKnob.setBounds(highCutoffKnob.getRight() + spacing, knobsY, knobSize, knobSize);
    highSlopeLabel.setBounds(
        highSlopeKnob.getX(),
        highSlopeKnob.getBottom(),
        knobSize,
        knobLabelHeight
    );

    // High Emphasis Knob
    highEmphasisKnob.setBounds(highSlopeKnob.getRight() + spacing, knobsY, knobSize, knobSize);
    highEmphasisLabel.setBounds(
        highEmphasisKnob.getX(),
        highEmphasisKnob.getBottom(),
        knobSize,
        knobLabelHeight
    );

    // Add extra spacing between High-Pass and Low-Pass sections
    area.removeFromTop(sectionSpacing);

    // Second Section: Low-Pass
    auto lowPassArea = area.removeFromTop(sectionHeight);

    // Place Low-Pass Label
    lowPassLabel.setBounds(
        lowPassArea.getCentreX() - 53,
        lowPassArea.getY(),
        80,
        labelHeight
    );

    // Place Low-Pass ToggleButton
    lowToggle.setBounds(
        lowPassLabel.getRight() + spacing,
        lowPassArea.getY(),
        toggleSize,
        toggleSize
    );

    // Place Low-Pass Knobs and their Labels
    knobsStartX = lowPassArea.getCentreX() - (totalKnobWidth / 2);
    knobsY = lowPassLabel.getBottom() + spacing;

    // Low Cutoff Knob
    lowCutoffKnob.setBounds(knobsStartX, knobsY, knobSize, knobSize);
    lowCutoffLabel.setBounds(
        lowCutoffKnob.getX(),
        lowCutoffKnob.getBottom(),
        knobSize,
        knobLabelHeight
    );

    // Low Slope Knob
    lowSlopeKnob.setBounds(lowCutoffKnob.getRight() + spacing, knobsY, knobSize, knobSize);
    lowSlopeLabel.setBounds(
        lowSlopeKnob.getX(),
        lowSlopeKnob.getBottom(),
        knobSize,
        knobLabelHeight
    );

    // Low Emphasis Knob
    lowEmphasisKnob.setBounds(lowSlopeKnob.getRight() + spacing, knobsY, knobSize, knobSize);
    lowEmphasisLabel.setBounds(
        lowEmphasisKnob.getX(),
        lowEmphasisKnob.getBottom(),
        knobSize,
        knobLabelHeight
    );
}