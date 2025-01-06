/*
  ==============================================================================

    Synth.cpp
    Created: 5 Jan 2025 4:11:20pm
    Author:  mikey

  ==============================================================================
*/

#include <JuceHeader.h>
#include "Synth.h"

//==============================================================================
Synth::Synth()
{
    for (int i = 0; i < 8; ++i)
        addVoice(new juce::SamplerVoice());
}

void Synth::loadSampleForNote(const juce::String& filePath, int midiNoteNumber)
{
    std::unique_ptr<juce::AudioFormatReader> reader(formatManager.createReaderFor(juce::File(filePath)));

    if (reader != nullptr)
    {
        juce::BigInteger midiNotes;
        midiNotes.setBit(midiNoteNumber);

        const double attackTimeSecs = 0.0;
        const double releaseTimeSecs = 0.1;
        const double maxSampleLengthSeconds = 10.0;

        addSound(new juce::SamplerSound(
            "sample" + juce::String(midiNoteNumber),
            *reader,
            midiNotes,
            midiNoteNumber,
            attackTimeSecs,
            releaseTimeSecs,
            maxSampleLengthSeconds
        ));
    }
}

void Synth::initializeFormatManager()
{
    formatManager.registerBasicFormats();
}


