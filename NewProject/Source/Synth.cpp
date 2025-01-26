/*
  ==============================================================================

    Synth.cpp
    Created: 5 Jan 2025 4:11:20pm
    Author:  mikey

  ==============================================================================
*/

#include <JuceHeader.h>
#include "Synth.h"
#include "BinaryData.h"

//==============================================================================
Synth::Synth()
{
	formatManager.registerFormat(new juce::WavAudioFormat(), true);
    for (int i = 0; i < 8; ++i)
        addVoice(new juce::SamplerVoice());
}

void Synth::loadSamples()
{
    clearSounds();

    // Print out all available resource names
    DBG("CHECK Available resources:");
    for (int i = 0; i < BinaryData::namedResourceListSize; ++i)
    {
        DBG(BinaryData::namedResourceList[i]);
    }

    for (int midiNote = 21; midiNote <= 108; ++midiNote)
    {
        juce::String fileName = "_" + juce::String(midiNote) + "_wav";
        int size = 0;

        // Load the sample data from BinaryData
        if (auto* data = BinaryData::getNamedResource(fileName.toRawUTF8(), size))
        {
            // Create a MemoryInputStream for the sample data
            auto inputStream = std::make_unique<juce::MemoryInputStream>(data, static_cast<size_t>(size), false);

            // Create an AudioFormatReader for the sample data
            auto formatReader = std::unique_ptr<juce::AudioFormatReader>(formatManager.createReaderFor(std::move(inputStream)));

            if (formatReader != nullptr)
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
                    0.1,   // Attack time
                    0.1,   // Release time
                    10.0)); // Maximum sample length

                // The formatReader will be automatically deleted when it goes out of scope
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

