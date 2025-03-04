/*
  ==============================================================================

    FDNReverb.cpp
    Created: 18 Feb 2025 10:24:35am
    Author:  mikey

  ==============================================================================
*/

#include "FDNReverb.h"

FDNReverb::FDNReverb(){

}

FDNReverb::~FDNReverb() {
}

void FDNReverb::process(juce::AudioBuffer<float>& buffer, double predelay, double decay, double diffusion, double dryWet) {
	DBG("Values: " << predelay << " " << decay << " " << diffusion << " " << dryWet);

        for (int channel = 0; channel < buffer.getNumChannels(); ++channel)
    {
        float* channelData = buffer.getWritePointer(channel);
        
        for (int sample = 0; sample < buffer.getNumSamples(); ++sample)
        {
            // Apply your reverb processing here
            // This is just a placeholder - implement your actual reverb algorithm
            channelData[sample] = channelData[sample] * (1.0f - dryWet) + 
                                 (channelData[sample] * decay * diffusion) * dryWet;
        }
    }
}