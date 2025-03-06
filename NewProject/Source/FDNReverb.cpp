/*
  ==============================================================================

    FDNReverb.cpp
    Created: 18 Feb 2025 10:24:35am
    Author:  mikey

  ==============================================================================
*/

#include "FDNReverb.h"

// AudioPluginHost set at 512 numSamples

FDNReverb::FDNReverb(){

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

    auto hadamardOutput = hadamard(outputs);
    return hadamardOutput;
}

// Applies a 4x4 Hadamard mixing matrix to the delay outputs.
// 'delayOutputs' is assumed to have 4 channels (delay lines),
// and each channel has the same number of samples.
std::vector<std::vector<float>> FDNReverb::hadamard(const std::vector<std::vector<float>>& delayOutputs)
{
    int numSamples = delayOutputs[0].size();

    // Prepare the output container: one vector per channel.
    std::vector<std::vector<float>> mixedOutputs(numDelayLines, std::vector<float>(numSamples, 0.0f));

    // Process each sample index
    for (int sample = 0; sample < numSamples; ++sample)
    {
        // Form a temporary vector containing the sample from each delay line
        std::array<float, numDelayLines> sampleVec;
        for (int ch = 0; ch < numDelayLines; ++ch)
        {
            sampleVec[ch] = delayOutputs[ch][sample];
        }

        // Multiply the sample vector by the Hadamard matrix:
        // For each output channel, compute the dot product of the corresponding row of the matrix with sampleVec.
        std::array<float, numDelayLines> mixedSample = { 0.0f, 0.0f, 0.0f, 0.0f };
        for (int row = 0; row < numDelayLines; ++row)
        {
            for (int col = 0; col < numDelayLines; ++col)
            {
                mixedSample[row] += hadamardMatrix[row][col] * sampleVec[col];
            }
        }

        // Store the mixed sample back to each channel
        for (int row = 0; row < numDelayLines; ++row)
        {
            mixedOutputs[row][sample] = mixedSample[row];
        }
    }
    return mixedOutputs;
}