/*
  ==============================================================================

    Synth.cpp
    Created: 5 Jan 2025 4:11:20pm
    Author:  mikey

  ==============================================================================
*/

#include "Synth.h"

//==============================================================================
Synth::Synth()
{
	formatManager.registerFormat(new juce::WavAudioFormat(), true);
    for (int i = 0; i < 16; ++i) {
        addVoice(new CustomSamplerVoice());
    }
}

void Synth::loadSamples()
{
    clearSounds();

    for (int midiNote = 24; midiNote <= 101; ++midiNote)
    {
        juce::String fileName = "_" + juce::String(midiNote) + "_wav";
        int size = 0;

        // Load the sample data from BinaryData
        if (auto* data = BinaryData::getNamedResource(fileName.toRawUTF8(), size))
        {
            // Create a MemoryInputStream for the sample data
            auto inputStream = std::make_unique<juce::MemoryInputStream>(data, static_cast<size_t>(size), false);

            // Create an AudioFormatReader for the sample data

            if (auto* formatReader = formatManager.createReaderFor(std::move(inputStream)))
            {
                // Create a BigInteger and set the bit for the current MIDI note
                juce::BigInteger allNotes;
                allNotes.setBit(midiNote);

                // Add the sample to the synthesizer with the specified MIDI note
                addSound(new juce::SamplerSound(
                    fileName,
                    *formatReader,
                    allNotes,
                    midiNote,
                    0.0,   // Attack time
                    0.0,   // Release time
                    10.0)); // Maximum sample length
                
                delete formatReader;
            }
            else
            {
                // Handle the case where the reader could not be created
                DBG("Failed to create AudioFormatReader for " << fileName);
            }
        }
        else
        {
            // Handle the case where the sample data could not be loaded
            DBG("Failed to load sample data for " << fileName);
        }
    }
}

