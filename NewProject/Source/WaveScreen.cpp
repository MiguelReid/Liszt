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
bool WaveScreen::isScreenEnabled = false;

WaveScreen::WaveScreen()
{
    // AudioVisualizer
    audioVisualiser.setRepaintRate(30); // 30 frames/s
    audioVisualiser.setBufferSize(128);
    audioVisualiser.setSamplesPerBlock(16);
    audioVisualiser.setNumChannels(1);
    audioVisualiser.setColours(juce::Colours::black, juce::Colours::white);
    audioVisualiser.setEnabled(true);
    addAndMakeVisible(audioVisualiser);

    // Button
    toggleButtonLookAndFeel = std::make_unique<ToggleButton>();
    screenButton.setLookAndFeel(toggleButtonLookAndFeel.get());
    screenButton.setClickingTogglesState(true);
	addAndMakeVisible(screenButton);

    screenButton.onClick = [this]()
        {
            isScreenEnabled = screenButton.getToggleState();
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

    // Adjust the area for the audioVisualiser to make space for the button
    auto visualiserArea = area.withTrimmedRight(buttonWidth + 5); // 5 pixels padding
    audioVisualiser.setBounds(visualiserArea);

    // Position the button on the left side, centered vertically
	auto buttonY = (visualiserArea.getHeight() - buttonHeight) / 2 + 2; // 2 pixels offset
    screenButton.setBounds(visualiserArea.getRight(), buttonY, buttonWidth, buttonHeight);

}

void WaveScreen::pushBufferIntoVisualiser(const juce::AudioBuffer<float>& buffer)
{
    if (isScreenEnabled)
    {
        audioVisualiser.pushBuffer(buffer);
    }
}