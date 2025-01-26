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
#include "CircularBuffer.cpp"

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
    static bool getVisualiserStatus() { return isScreenEnabled; }

private:
    juce::ToggleButton screenButton;
    std::unique_ptr<ToggleButton> toggleButtonLookAndFeel;
    juce::AudioVisualiserComponent audioVisualiser{ 1 }; // mono audio
    CircularBuffer<float, 128> circularBuffer;

	static bool isScreenEnabled;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (WaveScreen)
};
