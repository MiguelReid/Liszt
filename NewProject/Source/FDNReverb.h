/*
  ==============================================================================

    FDNReverb.h
    Created: 18 Feb 2025 10:24:35am
    Author:  mikey

  ==============================================================================
*/

#pragma once
#include <JuceHeader.h>

class FDNReverb
{
public:
    FDNReverb();
    ~FDNReverb();

    void FDNReverb::process(juce::AudioBuffer<float>& buffer, double predelay, double decay, double diffusion, double dryWet);

private:
};