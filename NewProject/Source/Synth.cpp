/*
  ==============================================================================

    Synth.cpp
    Created: 5 Jan 2025 4:11:20pm
    Author:  mikey

  ==============================================================================
*/

#include <JuceHeader.h>
#include "Synth.h"
#include "BinaryData.h" // Include the BinaryData header if using embedded resources

//==============================================================================
Synth::Synth()
{
    formatManager.registerBasicFormats();
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
        // Generate the file name for the sample (this example uses a hardcoded file name)
        // juce::String fileName = "_60_mp3";

		juce::String fileName = "_" + juce::String(midiNote) + "_mp3";

        int size;
        // Load the sample data from BinaryData
        if (auto* data = BinaryData::getNamedResource(fileName.toRawUTF8(), size))
        {
            // Create a MemoryInputStream for the sample data
            auto inputStream = std::make_unique<juce::MemoryInputStream>(data, size, false);

            // Create an AudioFormatReader for the sample data
            if (auto formatReader = formatManager.createReaderFor(std::move(inputStream)))
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
                    0.1,
                    0.1,
                    10.0));
            }
        }
    }
}
