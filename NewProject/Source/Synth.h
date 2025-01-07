/*
  ==============================================================================

    Synth.h
    Created: 5 Jan 2025 4:11:20pm
    Author:  mikey

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

class Synth : public juce::Synthesiser
{
public:
    Synth();
    void loadSamples();

private:
    juce::AudioFormatManager formatManager;
};
