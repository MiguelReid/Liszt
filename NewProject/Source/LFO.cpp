/*
  ==============================================================================

    LFO.cpp
    Created: 17 Mar 2025 2:12:07pm
    Author:  mikey

  ==============================================================================
*/

#include "LFO.h"

LFO::LFO() : phase(0.0), sampleRate(44100.0)
{
}

LFO::~LFO()
{
}

float LFO::processLFO(double lfoDepth, int lfoShape, int boxIndex)
{
    // Frequency to control LFO's oscillation
    auto frequency = 5.0;

    // Depth scaling parameter dependant
    const std::array<float, 3> depthScales = { 1.0f, 5.0f, 100.0f };
    float depthScale = depthScales[std::clamp(boxIndex, 0, 2)];
    const float depth = static_cast<float>(lfoDepth) * depthScale;

    // Phase update
    phase += frequency / sampleRate;
    if (phase >= 1.0)
        phase -= 1.0;

    float lfoValue = 0.0f;

    switch (lfoShape)
    {
    case 0: // Sine
        lfoValue = std::sin(static_cast<float>(phase) * juce::MathConstants<float>::twoPi);
        break;

    case 1: // Triangle
        if (phase < 0.25)
            lfoValue = static_cast<float>(4.0 * phase);
        else if (phase < 0.75)
            lfoValue = static_cast<float>(2.0 - 4.0 * phase);
        else
            lfoValue = static_cast<float>(4.0 * phase - 4.0);
        lfoValue = lfoValue * 2.0f - 1.0f;
        break;

    case 2: // Square
        lfoValue = (phase < 0.5) ? -1.0f : 1.0f;
        break;

    default:
        break;
    }

    // Output
    float result = lfoValue * depth;

    return result;
}