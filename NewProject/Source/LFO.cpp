/*
  ==============================================================================

    LFO.cpp
    Created: 17 Mar 2025 2:12:07pm
    Author:  mikey

  ==============================================================================
*/

#include "LFO.h"

LFO::LFO() : phase(0.0f), sampleRate(44100.0)
{
}

LFO::~LFO()
{
}

std::vector<std::vector<float>> LFO::processLFO(juce::AudioBuffer<float>& buffer,
    double lfoRange,
    int lfoShape)
{
    const int numChannels = buffer.getNumChannels();
    const int numSamples = buffer.getNumSamples();

    // Create output buffers matching the input
    std::vector<std::vector<float>> outputs(numChannels, std::vector<float>(numSamples, 0.0f));

    // Map range parameter (0.0-1.0) to frequency (0.1-10Hz)
    const float frequency = 0.1f + (lfoRange * 9.9f);
    const float depth = 0.5f; // Fixed depth for modulation
    const double cyclesPerSample = frequency / sampleRate;

    for (int sample = 0; sample < numSamples; ++sample)
    {
        // Advance LFO phase
        phase += cyclesPerSample;
        if (phase >= 1.0f)
            phase -= 1.0f;

        // Calculate LFO value based on selected waveform
        float lfoValue = 0.0f;
        switch (lfoShape)
        {
        case 0: // Sine
            lfoValue = std::sin(phase * juce::MathConstants<float>::twoPi) * depth;
            break;

        case 1: // Triangle
            lfoValue = (phase < 0.5f ?
                phase * 4.0f - 1.0f :
                3.0f - phase * 4.0f) * depth;
            break;

        case 2: // Square
            lfoValue = (phase < 0.5f ? 1.0f : -1.0f) * depth;
            break;
        }

        // Apply LFO to all channels and copy to output
        for (int channel = 0; channel < numChannels; ++channel)
        {
            float* channelData = buffer.getWritePointer(channel);
            float modulated = channelData[sample] * (1.0f + lfoValue);

            // Store modulated sample in both the buffer and our output vector
            channelData[sample] = modulated;
            outputs[channel][sample] = modulated;
        }
    }

    return outputs;
}