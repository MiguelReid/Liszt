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
    addAndMakeVisible(arpeggiatorButton);
    addAndMakeVisible(pitchBendSlider);

    arpeggiatorButton.setButtonText("Arp");

    toggleButtonLookAndFeel = std::make_unique<ToggleButton>();
    arpeggiatorButton.setLookAndFeel(toggleButtonLookAndFeel.get());
    //arpegiattorButton.setToggleState(false, juce::NotificationType::dontSendNotification);
    pitchBendSlider.setSliderStyle(juce::Slider::SliderStyle::LinearBarVertical);
}

LeftControls::~LeftControls()
{
    arpeggiatorButton.setLookAndFeel(nullptr);
    setLookAndFeel(nullptr);
    knobLookAndFeel.reset();
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
    // Bottom elements height
    arpeggiatorButton.setBounds(bounds.removeFromBottom(70).reduced(10));
    pitchBendSlider.setBounds(bounds.removeFromBottom(30).reduced(10));
}