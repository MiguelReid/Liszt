/*
  ==============================================================================

    Synth.h
    Created: 5 Jan 2025 4:11:20pm
    Author:  mikey

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

//==============================================================================
/*
*/
class Synth : public juce::Synthesiser
{
public:
    Synth();

    void loadSampleForNote(const juce::String& filePath, int midiNoteNumber);
    void initializeFormatManager();

private:
    juce::AudioFormatManager formatManager;

};
