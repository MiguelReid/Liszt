/*
  ==============================================================================

    OscillatorControls.cpp
    Created: 13 Jan 2025 12:48:25pm
    Author:  mikey

  ==============================================================================
*/

#include "OscillatorControls.h"

//==============================================================================
OscillatorControls::OscillatorControls(juce::AudioProcessorValueTreeState& apvts) : apvts(apvts) {

    // Initialize and configure the label
    oscillatorLabel.setText("Oscillators", juce::dontSendNotification);
    oscillatorLabel.setJustificationType(juce::Justification::centred);
    oscillatorLabel.setFont(juce::Font(18.0f, juce::Font::bold));
    oscillatorLabel.setColour(juce::Label::textColourId, juce::Colours::white);
    addAndMakeVisible(oscillatorLabel);

    // Initialize and configure rangeKnob1
    rangeKnob1.setSliderStyle(juce::Slider::Rotary);
    rangeKnob1.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    addAndMakeVisible(rangeKnob1);
    rangeLabel1.setText("Range", juce::dontSendNotification);
    rangeLabel1.setJustificationType(juce::Justification::centredTop);
    rangeLabel1.setFont(12.0f);
    rangeLabel1.setColour(juce::Label::textColourId, juce::Colours::white);
    addAndMakeVisible(rangeLabel1);

    // Label for Oscillator 1
    oscLabel1.setText("Osc 1", juce::dontSendNotification);
    oscLabel1.setJustificationType(juce::Justification::centred);
    oscLabel1.setFont(14.0f);
    oscLabel1.setColour(juce::Label::textColourId, juce::Colours::white);
    addAndMakeVisible(oscLabel1);

    // Initialize and configure shapeKnob1
    shapeKnob1.setSliderStyle(juce::Slider::Rotary);
    shapeKnob1.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    shapeKnob1.setRange(0, 2, 1); // Only three positions: 0, 1, 2
    // Set rotary parameters to reduce angular range
    shapeKnob1.setRotaryParameters(juce::MathConstants<float>::pi / 6.0f,
        juce::MathConstants<float>::pi / 2.0f,
        true);
    addAndMakeVisible(shapeKnob1);

    shapeLabel1.setText("Shape", juce::dontSendNotification);
    shapeLabel1.setJustificationType(juce::Justification::centredTop);
    shapeLabel1.setFont(12.0f);
    shapeLabel1.setColour(juce::Label::textColourId, juce::Colours::white);
    addAndMakeVisible(shapeLabel1);

    // Initialize and configure rangeKnob2
    rangeKnob2.setSliderStyle(juce::Slider::Rotary);
    rangeKnob2.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    addAndMakeVisible(rangeKnob2);
    rangeLabel2.setText("Range", juce::dontSendNotification);
    rangeLabel2.setJustificationType(juce::Justification::centredTop);
    rangeLabel2.setFont(12.0f);
    rangeLabel2.setColour(juce::Label::textColourId, juce::Colours::white);
    addAndMakeVisible(rangeLabel2);

    // Label for Oscillator 2
    oscLabel2.setText("Osc 2", juce::dontSendNotification);
    oscLabel2.setJustificationType(juce::Justification::centred);
    oscLabel2.setFont(14.0f);
    oscLabel2.setColour(juce::Label::textColourId, juce::Colours::white);
    addAndMakeVisible(oscLabel2);

    // Initialize and configure shapeKnob2
    shapeKnob2.setSliderStyle(juce::Slider::Rotary);
    shapeKnob2.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    shapeKnob2.setRange(0, 2, 1); // Only three positions: 0, 1, 2
    // Set rotary parameters to reduce angular range
    shapeKnob2.setRotaryParameters(juce::MathConstants<float>::pi / 6.0f,
        juce::MathConstants<float>::pi / 2.0f,
        true);
    addAndMakeVisible(shapeKnob2);

    shapeLabel2.setText("Shape", juce::dontSendNotification);
    shapeLabel2.setJustificationType(juce::Justification::centredTop);
    shapeLabel2.setFont(12.0f);
    shapeLabel2.setColour(juce::Label::textColourId, juce::Colours::white);
    addAndMakeVisible(shapeLabel2);

    addAndMakeVisible(osc1Button);
    addAndMakeVisible(osc2Button);

    // =================== Look and feel ========================

    toggleButtonLookAndFeel = std::make_unique<ToggleButton>();
    knobLookAndFeel = std::make_unique<Knob>();

	shapeKnob1.setName("ShapeKnob");
	shapeKnob2.setName("ShapeKnob");
    rangeKnob1.setLookAndFeel(knobLookAndFeel.get());
    rangeKnob2.setLookAndFeel(knobLookAndFeel.get());
    shapeKnob1.setLookAndFeel(knobLookAndFeel.get());
    shapeKnob2.setLookAndFeel(knobLookAndFeel.get());
    osc1Button.setLookAndFeel(toggleButtonLookAndFeel.get());
    osc2Button.setLookAndFeel(toggleButtonLookAndFeel.get());

	// AudioProcessorValueTreeState ===============================
	rangeAttachment1 = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
		apvts, "OSC1_RANGE", rangeKnob1);
	shapeAttachment1 = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
		apvts, "OSC1_SHAPE", shapeKnob1);
	rangeAttachment2 = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
		apvts, "OSC2_RANGE", rangeKnob2);
	shapeAttachment2 = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
		apvts, "OSC2_SHAPE", shapeKnob2);
	oscAttachment1 = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(
		apvts, "OSC1_ENABLED", osc1Button);
	oscAttachment2 = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(
		apvts, "OSC2_ENABLED", osc2Button);

}

OscillatorControls::~OscillatorControls()
{
    rangeKnob1.setLookAndFeel(nullptr);
    rangeKnob2.setLookAndFeel(nullptr);
    shapeKnob1.setLookAndFeel(nullptr);
    shapeKnob2.setLookAndFeel(nullptr);
    osc1Button.setLookAndFeel(nullptr);
    osc2Button.setLookAndFeel(nullptr);
}

void OscillatorControls::paint(juce::Graphics& g)
{
}

void OscillatorControls::resized()
{
    auto bounds = getLocalBounds();
    auto labelHeight = 20;
    auto knobWidth = 55;
    auto knobHeight = 55;
    auto marginX = 10;
    auto marginY = 10;
    auto labelWidth = 50;
    auto buttonWidth = 20;
    auto buttonHeight = 20;

    // Calculate total width for the three columns
    auto totalWidth = 3 * knobWidth + 2 * marginX + buttonWidth;

    // Position the label at the top, centered
    oscillatorLabel.setBounds((bounds.getWidth() - totalWidth) / 2, 0, totalWidth, labelHeight);
    bounds.removeFromTop(labelHeight);

    // First Row
    int startX = (getWidth() - totalWidth) / 2;
    int startY = bounds.getY() + marginY;

    // rangeKnob1
    rangeKnob1.setBounds(startX, startY, knobWidth, knobHeight);
    rangeLabel1.setBounds(rangeKnob1.getX(), rangeKnob1.getBottom(), knobWidth, 15);

    // oscLabel1
    oscLabel1.setBounds(rangeKnob1.getRight() + marginX, startY + (knobWidth / 2) - 10, knobWidth, 20);

    // osc1Button
    osc1Button.setBounds(oscLabel1.getRight() - 5, startY + (knobWidth / 2) - 10, buttonWidth, buttonHeight);

    // shapeKnob1 with increased height
    shapeKnob1.setBounds(osc1Button.getRight() + marginX, startY, knobWidth, knobHeight);
    shapeLabel1.setBounds(shapeKnob1.getX(), shapeKnob1.getBottom(), knobWidth, 15);

    // Second Row
    startY = shapeLabel1.getBottom() + marginY;  // Adjust startY based on shapeKnob1's bottom

    // rangeKnob2
    rangeKnob2.setBounds(startX, startY, knobWidth, knobHeight);
    rangeLabel2.setBounds(rangeKnob2.getX(), rangeKnob2.getBottom(), knobWidth, 15);

    // oscLabel2
    oscLabel2.setBounds(rangeKnob2.getRight() + marginX, startY + (knobWidth / 2) - 10, knobWidth, 20);

    // osc2Button
    osc2Button.setBounds(oscLabel2.getRight() - 5, startY + (knobWidth / 2) - 10, buttonWidth, buttonHeight);

    // shapeKnob2 with increased height
    shapeKnob2.setBounds(osc2Button.getRight() + marginX, startY, knobWidth, knobHeight);
    shapeLabel2.setBounds(shapeKnob2.getX(), shapeKnob2.getBottom(), knobWidth, 15);
}




