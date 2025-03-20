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

// Returns a modulation value for the specified target parameter
float LFO::processLFO(double lfoDepth, int lfoShape, int boxIndex)
{
    // Faster frequency for more rapid oscillation
    auto frequency = 5.0f;

    // More aggressive depth scaling based on target parameter
    // These values determine the maximum possible range of modulation when depth = 1.0
    const std::array<float, 4> depthScales = { 1.0f, 4.2f, 100.0f, 2.0f };
    float depthScale = depthScales[std::clamp(boxIndex, 0, 3)];

    // Apply depth control directly - linear scaling works better for predictable ranges
    const float depth = static_cast<float>(lfoDepth) * depthScale;

    // Phase with precise floating point
    phase += frequency / sampleRate;
    if (phase >= 1.0f)
        phase -= 1.0f;

    // Calculate LFO value based on selected waveform
    float lfoValue = 0.0f;
    switch (lfoShape)
    {
    case 0: // Sine - pure bipolar sine wave (-1 to +1)
        lfoValue = std::sin(phase * juce::MathConstants<double>::twoPi);
        break;

    case 1: // Triangle - bipolar triangle wave (-1 to +1)
        lfoValue = phase < 0.5f
            ? (4.0f * phase - 1.0f)       // -1 to +1 for first half
            : (3.0f - 4.0f * phase);      // +1 to -1 for second half
        break;

    case 2: // Square - bipolar with anti-aliasing (-1 to +1)
    {
        constexpr float smoothFactor = 0.01f; // Sharper transitions
        const float threshold = 0.5f;

        if (std::abs(phase - threshold) < smoothFactor) {
            // Smooth transition using sigmoid-like function
            float normalized = (phase - (threshold - smoothFactor)) / (2.0f * smoothFactor);
            lfoValue = normalized > 0.5f ? 1.0f : -1.0f;
            lfoValue *= (std::abs(normalized - 0.5f) * 2.0f); // Smooth the transition
        }
        else {
            lfoValue = phase < threshold ? -1.0f : 1.0f;
        }
    }
    break;
    }

    // All outputs are now directly bipolar (-1 to +1) and scaled by depth
    // This makes the depth directly control the maximum deviation from center
    return lfoValue * depth;
}


