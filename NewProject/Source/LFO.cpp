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

    // Use lfoRange directly as frequency in Hz (0-30Hz range from plugin parameters)
    const float frequency = juce::jlimit(0.01f, 30.0f, static_cast<float>(lfoRange));

    // Make depth a parameter you can adjust based on desired intensity
    const float depth = 0.05f;

    // Calculate phase increment per sample with higher precision
    const double phaseIncrement = frequency / sampleRate;

    for (int sample = 0; sample < numSamples; ++sample)
    {
        // Advance LFO phase with precise floating point
        phase += phaseIncrement;
        if (phase >= 1.0f)
            phase -= 1.0f;

        // Calculate LFO value based on selected waveform
        float lfoValue = 0.0f;
        switch (lfoShape)
        {
        case 0: // Sine - optimized sine calculation
            lfoValue = 0.5f * (std::sin(phase * juce::MathConstants<double>::twoPi) + 1.0f);
            break;

        case 1: // Triangle - more efficient implementation
            lfoValue = phase < 0.5f ? 2.0f * phase : 2.0f * (1.0f - phase);
            break;

        case 2: // Square - with improved anti-aliasing
        {
            constexpr float smoothFactor = 0.005f; // Smaller for sharper transitions
            const float threshold = 0.5f;

            if (std::abs(phase - threshold) < smoothFactor) {
                // Smooth transition using sigmoid-like function
                float normalized = (phase - (threshold - smoothFactor)) / (2.0f * smoothFactor);
                lfoValue = normalized > 0.5f ? 1.0f - (1.0f - normalized) * (1.0f - normalized) : normalized * normalized;
            }
            else {
                lfoValue = phase < threshold ? 0.0f : 1.0f;
            }
        }
        break;
        }

        // Map LFO to proper range for audio multiplication: 1.0 - depth to 1.0 + depth
        lfoValue = 1.0f + (lfoValue * 2.0f - 1.0f) * depth;

        // Apply LFO to all channels
        for (int channel = 0; channel < numChannels; ++channel)
        {
            float* channelData = buffer.getWritePointer(channel);

            // Multiplicative modulation for natural sounding LFO effects
            float modulated = channelData[sample] * lfoValue;

            // Store modulated sample in both buffer and output vector
            channelData[sample] = modulated;
            outputs[channel][sample] = modulated;
        }
    }

    return outputs;
}

