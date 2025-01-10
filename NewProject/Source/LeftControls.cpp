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

    arpeggiatorButton.setSize(40, 60);  // Adjust size as needed
	pitchBendSlider.setSize(20, 50);  // Adjust size as needed
	gainSlider.setSize(20, 50);  // Adjust size as needed

    arpeggiatorButton.setButtonText("Arp");
    pitchBendSlider.setName("Pitch Bend");
    gainSlider.setName("Gain");

	arpeggiatorButton.setClickingTogglesState(true);

    pitchBendSlider.setSliderStyle(juce::Slider::SliderStyle::LinearBarVertical);
    gainSlider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);

    pitchBendSlider.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    gainSlider.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);

    // Initialize and configure labels
    pitchBendLabel.setText("Pitch Bend", juce::dontSendNotification);
    pitchBendLabel.setJustificationType(juce::Justification::centredTop);
    pitchBendLabel.setFont(12.0f);
    pitchBendLabel.setColour(juce::Label::textColourId, juce::Colours::white);

    gainLabel.setText("Gain", juce::dontSendNotification);
    gainLabel.setJustificationType(juce::Justification::centredTop);
    gainLabel.setFont(12.0f);
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

    // Top section height (e.g., 30% of the total height)
    int topSectionHeight = bounds.proportionOfHeight(0.45f);

    // Position the arpeggiator button at the top center
    arpeggiatorButton.setCentrePosition(bounds.getCentreX(), topSectionHeight / 2);

    // Bottom section bounds
    auto bottomSectionBounds = bounds.withTop(topSectionHeight);

    // Split bottom section into left and right halves
    auto leftHalf = bottomSectionBounds.removeFromLeft(bottomSectionBounds.getWidth() / 2);
    auto rightHalf = bottomSectionBounds;

    // Adjust slider sizes if necessary
    int sliderWidth = 20;
    int sliderHeight = 60;

    // Position pitch bend slider in the left half
    pitchBendSlider.setBounds(leftHalf.getCentreX() - sliderWidth / 2,
        leftHalf.getCentreY() - sliderHeight / 2 - 10,  // Adjust vertical position
        sliderWidth, sliderHeight);

    // Position pitch bend label below the slider
    pitchBendLabel.setBounds(pitchBendSlider.getX() - 10,
        pitchBendSlider.getBottom() + 5,
        sliderWidth + 20, 20);

    // Position gain slider in the right half
    gainSlider.setBounds(rightHalf.getCentreX() - sliderWidth / 2,
        rightHalf.getCentreY() - sliderHeight / 2 - 10,  // Adjust vertical position
        sliderWidth, sliderHeight);

    // Position gain label below the slider
    gainLabel.setBounds(gainSlider.getX() - 10,
        gainSlider.getBottom() + 5,
        sliderWidth + 20, 20);
}