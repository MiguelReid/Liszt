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

    // Visualiser
    void pushBufferIntoVisualiser(const juce::AudioBuffer<float>& buffer);
    static bool getVisualiserStatus() { return isScreenEnabled; }
    juce::AudioVisualiserComponent audioVisualiser{ 1 }; // mono audio

private:
    juce::ToggleButton screenButton;
    std::unique_ptr<ToggleButton> toggleButtonLookAndFeel;

	static bool isScreenEnabled;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (WaveScreen)
};
