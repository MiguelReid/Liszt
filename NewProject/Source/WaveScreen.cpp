/*
  ==============================================================================

    WaveScreen.cpp
    Created: 28 Oct 2024 1:09:43pm
    Author:  mikey

  ==============================================================================
*/

#include <JuceHeader.h>
#include "WaveScreen.h"

//==============================================================================
WaveScreen::WaveScreen()
{
    // AudioVisualizer
    audioVisualiser.setBufferSize(128);
    audioVisualiser.setSamplesPerBlock(16);
    audioVisualiser.setColours(juce::Colours::black, juce::Colours::white);
    addAndMakeVisible(audioVisualiser);

    // Button
    toggleButtonLookAndFeel = std::make_unique<ToggleButton>();
    screenButton.setLookAndFeel(toggleButtonLookAndFeel.get());
    screenButton.setClickingTogglesState(true);
	addAndMakeVisible(screenButton);

    screenButton.onClick = [this]()
        {
            bool isEnabled = screenButton.getToggleState();
            audioVisualiser.setEnabled(isEnabled);
        };
}

WaveScreen::~WaveScreen()
{
    screenButton.setLookAndFeel(nullptr);
}

void WaveScreen::paint(juce::Graphics& g)
{

}

void WaveScreen::resized()
{
    // Get the total area of the WaveScreen component
    auto area = getLocalBounds();

    // Define the dimensions of the button
    auto buttonWidth = 35; // Width of the button
    auto buttonHeight = 35; // Height of the button

    // Position the button on the left side, centered vertically
    auto buttonY = (area.getHeight() - buttonHeight) / 2;
    screenButton.setBounds(0, buttonY, buttonWidth, buttonHeight);

    // Adjust the area for the audioVisualiser to make space for the button
    auto visualiserArea = area.withTrimmedLeft(buttonWidth + 5); // 5 pixels padding
    audioVisualiser.setBounds(visualiserArea);
}

void WaveScreen::pushNextSampleIntoVisualiser(const float* samples, int numChannels)
{
    if (audioVisualiser.isEnabled())
        audioVisualiser.pushSample(samples, numChannels);
}
