/*
  ==============================================================================

    LeftControls.cpp
    Created: 5 Jan 2025 1:46:03pm
    Author:  mikey

  ==============================================================================
*/

#include "LeftControls.h"

//==============================================================================
LeftControls::LeftControls(juce::AudioProcessorValueTreeState& apvts) : apvts(apvts) {
    // Initialize LookAndFeel
    toggleButtonLookAndFeel = std::make_unique<ToggleButton>();
    knobLookAndFeel = std::make_unique<Knob>();

    arpeggiatorButton.setLookAndFeel(toggleButtonLookAndFeel.get());
	gainSlider.setLookAndFeel(knobLookAndFeel.get());

	arpeggiatorButton.setClickingTogglesState(true);

    pitchBendSlider.setSliderStyle(juce::Slider::SliderStyle::LinearBarVertical);
    gainSlider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);

    pitchBendSlider.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    gainSlider.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);

    // Initialize and configure labels
    pitchBendLabel.setText("Pitch", juce::dontSendNotification);
    pitchBendLabel.setJustificationType(juce::Justification::centredTop);
    pitchBendLabel.setColour(juce::Label::textColourId, juce::Colours::white);

    gainLabel.setText("Gain", juce::dontSendNotification);
    gainLabel.setJustificationType(juce::Justification::centredTop);
    gainLabel.setColour(juce::Label::textColourId, juce::Colours::white);

    arpeggiatorLabel.setText("Arp", juce::dontSendNotification);
    arpeggiatorLabel.setJustificationType(juce::Justification::centredTop);
    arpeggiatorLabel.setColour(juce::Label::textColourId, juce::Colours::white);

	// ==========================

    addAndMakeVisible(pitchBendLabel);
    addAndMakeVisible(gainLabel);
	addAndMakeVisible(arpeggiatorLabel);

    addAndMakeVisible(arpeggiatorButton);
    addAndMakeVisible(pitchBendSlider);
    addAndMakeVisible(gainSlider);

	// ==========================

	gainSlider.setSkewFactor(0.5f);

	// AudioProcessorValueTreeState ===============================
	pitchBendAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
		apvts, "PITCH_BEND", pitchBendSlider);
	gainAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
		apvts, "GAIN", gainSlider);
	arpeggiatorAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(apvts, "ARPEGGIATOR", arpeggiatorButton);
}

LeftControls::~LeftControls()
{
    arpeggiatorButton.setLookAndFeel(nullptr);
	gainSlider.setLookAndFeel(nullptr);
}

void LeftControls::paint (juce::Graphics& g)
{
    // Black Background
    g.fillAll(juce::Colours::black);

	// Wood-like Gradient
    juce::ColourGradient gradient(juce::Colour::fromRGB(30, 30, 30), 0, 0,
        juce::Colour::fromRGB(60, 60, 60), getWidth(), getHeight(), false);
    g.setGradientFill(gradient);
    g.fillRect(getLocalBounds());
}

void LeftControls::resized()
{
    auto bounds = getLocalBounds();
    const int labelHeight = 15;

    // Define dimensions for the arpeggiator button
    const int buttonWidth = 35;
    const int buttonHeight = 35;
    const int buttonMarginTop = 2;

    // Position the arpeggiator button at the top center
    arpeggiatorButton.setBounds(
        bounds.getCentreX() - buttonWidth / 2,
        buttonMarginTop,
        buttonWidth,
        buttonHeight
    );

    // Position the arpeggiator label beneath the button
    arpeggiatorLabel.setBounds(
        bounds.getCentreX() - buttonWidth / 2,
        buttonHeight - 5,
        buttonWidth,
        labelHeight
    );

    // Adjust dimensions for sliders and labels
    const int sliderHeight = 40;
    const int sliderWidth = 40;

    // Position pitch bend slider in the bottom left
    pitchBendSlider.setBounds(
        25,
        bounds.getBottom() - sliderHeight - labelHeight - 20,
        sliderWidth-20,
        sliderHeight+5
    );

    // Position pitch bend label beneath the slider
    pitchBendLabel.setBounds(
        15,
        bounds.getBottom() - labelHeight - 8,
        sliderWidth,
        labelHeight
    );

    // Position gain slider in the bottom right
    gainSlider.setBounds(
        bounds.getWidth() - sliderWidth - 15,
        bounds.getBottom() - sliderHeight - labelHeight - 8,
        sliderWidth,
        sliderHeight
    );

    // Position gain label beneath the slider
    gainLabel.setBounds(
        bounds.getWidth() - sliderWidth - 15,
        bounds.getBottom() - labelHeight - 8,
        sliderWidth,
        labelHeight
    );
}
