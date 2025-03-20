/*
  ==============================================================================

    LFO.h
    Created: 17 Mar 2025 2:12:07pm
    Author:  mikey

  ==============================================================================
*/

#pragma once
#include <JuceHeader.h>
#include <algorithm>

class LFO
{
public:
    LFO();
    ~LFO();

    float processLFO(double lfoRange, int lfoShape, int boxIndex);


    void setSampleRate(double newSampleRate) { sampleRate = newSampleRate; }

private:
    float phase = 0.0f;
    double sampleRate = 44100.0;
};