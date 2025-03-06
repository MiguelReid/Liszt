/*
  ==============================================================================

    FDNReverb.cpp
    Created: 18 Feb 2025 10:24:35am
    Author:  mikey

  ==============================================================================
*/

#include "FDNReverb.h"

// AudioPluginHost set at 512 numSamples

FDNReverb::FDNReverb() : numDelayLines(4){

    const int primeDelays[4] = { 1031, 1327, 1523, 1871 };

    for (int i = 0; i < numDelayLines; ++i) {
        //std::cout << "Delay line " << i << " created with delay of " << randomDelay << " samples" << std::endl;
        delayLines.push_back(std::make_unique<CustomDelayLine>(primeDelays[i]));
    }
}

FDNReverb::~FDNReverb() {
}

std::vector<std::vector<float>> FDNReverb::process(juce::AudioBuffer<float>& buffer, double predelay, double decay, double diffusion, double dryWet) {
    const int numChannels = buffer.getNumChannels();
    const int numSamples = buffer.getNumSamples();

    std::vector<std::vector<float>> outputs(numDelayLines, std::vector<float>(numSamples, 0.0f));

    for (int sample = 0; sample < numSamples; ++sample) {
        float inputSample = buffer.getSample(0, sample);
        for (int i = 0; i < numDelayLines; ++i) {
            outputs[i][sample] = delayLines[i]->processSample(inputSample);
        }
    }
    return outputs;
}