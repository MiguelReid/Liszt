/*
  ==============================================================================

    LeftControls.cpp
    Created: 5 Jan 2025 1:46:03pm
    Author:  mikey

  ==============================================================================
*/

#include "LeftControls.h"

//==============================================================================
LeftControls::LeftControls()
{
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

    // --------------------------

    addAndMakeVisible(pitchBendLabel);
    addAndMakeVisible(gainLabel);

    addAndMakeVisible(arpeggiatorButton);
    addAndMakeVisible(pitchBendSlider);
    addAndMakeVisible(gainSlider);
}

LeftControls::~LeftControls()
{
    arpeggiatorButton.setLookAndFeel(nullptr);
	gainSlider.setLookAndFeel(nullptr);
}

void LeftControls::paint (juce::Graphics& g)
{
    // Fondo negro
    g.fillAll(juce::Colours::black);

    // Ar un gradiente para simular un acabado de madera
    juce::ColourGradient gradient(juce::Colour::fromRGB(30, 30, 30), 0, 0,
        juce::Colour::fromRGB(60, 60, 60), getWidth(), getHeight(), false);
    g.setGradientFill(gradient);
    g.fillRect(getLocalBounds());
}

void LeftControls::resized()
{
    auto bounds = getLocalBounds();

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

    // Calculate the remaining space, ensuring room for labels at bottom
    auto slidersArea = bounds;
    slidersArea.removeFromTop(arpeggiatorButton.getBottom() + 5);
    slidersArea.removeFromBottom(25); // Reserve space for labels at bottom

    // Adjust dimensions for sliders and labels
    const int labelHeight = 15;
    const int sliderHeight = slidersArea.getHeight() + 5; // Leave some margin
    const int sliderWidth = 30;
    const int labelWidth = 40;

    // Split the area into left and right halves
    auto leftHalf = slidersArea.removeFromLeft(slidersArea.getWidth() / 2);
    auto rightHalf = slidersArea;

    // Set font size for labels
    pitchBendLabel.setFont(juce::Font(13.0f, juce::Font::plain));
    gainLabel.setFont(juce::Font(13.0f, juce::Font::plain));

    // Position pitch bend slider in the left half
    pitchBendSlider.setBounds(
        leftHalf.getCentreX() - sliderWidth / 2,
        leftHalf.getY(),
        sliderWidth,
        sliderHeight
    );

    // Position gain slider in the right half
    gainSlider.setBounds(
        rightHalf.getCentreX() - sliderWidth / 2,
        rightHalf.getY(),
        sliderWidth+15,
        sliderHeight+15
    );

    // Position labels at the bottom of the bounds
    const int labelY = bounds.getBottom() - labelHeight - 5; // 5 pixels from bottom

    pitchBendLabel.setBounds(
        leftHalf.getCentreX() - labelWidth / 2,
        labelY,
        labelWidth,
        labelHeight
    );

    gainLabel.setBounds(
        rightHalf.getCentreX() - labelWidth / 2,
        labelY,
        labelWidth,
        labelHeight
    );
}