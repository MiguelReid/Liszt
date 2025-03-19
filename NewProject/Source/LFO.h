/*
  ==============================================================================

    LFO.h
    Created: 17 Mar 2025 2:12:07pm
    Author:  mikey

  ==============================================================================
*/

#pragma once
#include <JuceHeader.h>

class LFO
{
public:
    LFO();
    ~LFO();

    std::vector<std::vector<float>> processLFO(juce::AudioBuffer<float>& buffer,
        double lfoRange, int lfoShape, int boxIndex);


    void setSampleRate(double newSampleRate) { sampleRate = newSampleRate; }

private:
    float phase = 0.0f;
    double sampleRate = 44100.0;
};

