/*
  ==============================================================================

    WaveScreen.h
    Created: 28 Oct 2024 1:09:43pm
    Author:  mikey

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "ToggleButton.h"

//==============================================================================
/*
*/
class WaveScreen  : public juce::Component
{
public:
    WaveScreen();
    ~WaveScreen() override;

    void paint (juce::Graphics&) override;
    void resized() override;

    // Method to feed audio samples to the visualiser
    void pushNextSampleIntoVisualiser(const float* samples, int numChannels);

private:
    juce::ToggleButton screenButton;
    std::unique_ptr<ToggleButton> toggleButtonLookAndFeel;
    juce::AudioVisualiserComponent audioVisualiser{ 2 }; // Assuming stereo audio

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (WaveScreen)
};
