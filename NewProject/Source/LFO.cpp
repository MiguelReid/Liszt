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

float LFO::processLFO(double lfoDepth, int lfoShape, int boxIndex)
{
    // Fixed frequency for consistent oscillation
    auto frequency = 5.0f;

    // More aggressive depth scaling based on target parameter
    const std::array<float, 3> depthScales = { 1.0f, 5.0f, 100.0f};
    float depthScale = depthScales[std::clamp(boxIndex, 0, 2)];

    // Apply depth control directly
    const float depth = static_cast<float>(lfoDepth) * depthScale;

    // Phase update
    phase += frequency / sampleRate;
    if (phase >= 1.0f)
        phase -= 1.0f;

    // Calculate LFO value based on selected waveform
    float lfoValue = 0.0f;

    switch (lfoShape)
    {
    case 0: // Sine
        lfoValue = std::sin(phase * juce::MathConstants<float>::twoPi);
        break;

    case 1: // Triangle
        lfoValue = phase < 0.5f
            ? (4.0f * phase - 1.0f)
            : (3.0f - 4.0f * phase);
        break;

    case 2: // Square
    {
        // Simplest square wave implementation
        lfoValue = (phase < 0.5f) ? -1.0f : 1.0f;
    }
    break;
    }

    // Final output
    float result = lfoValue * depth;
    DBG("value=" << result);

    return result;
}




