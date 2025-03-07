/*
  ==============================================================================

    FDNReverb.cpp
    Created: 18 Feb 2025 10:24:35am
    Author:  mikey

  ==============================================================================
*/

#include "FDNReverb.h"
<<<<<<< Updated upstream
=======

// AudioPluginHost set at 512 numSamples

FDNReverb::FDNReverb() {
    const int primeDelays[4] = { 1031, 1327, 1523, 1871 };
    for (int i = 0; i < numDelayLines; ++i) {
        delayLines.push_back(std::make_unique<CustomDelayLine>(primeDelays[i]));
        diffusionFilters.push_back(AllPassFilter());
        lpfFilters.push_back(SimpleLPF());
    }
}

FDNReverb::~FDNReverb() {
}

std::vector<std::vector<float>> FDNReverb::process(juce::AudioBuffer<float>& buffer, double predelay, double decay, double diffusion, double dryWet) {
    const int numChannels = buffer.getNumChannels();
    const int numSamples = buffer.getNumSamples();

    // Convert parameters to usable values - be more conservative with values
    float decayGain = juce::jlimit(0.0f, 0.95f, static_cast<float>(decay));
    float diffusionCoeff = juce::jlimit(0.0f, 0.5f, static_cast<float>(diffusion));
    float lpfCutoff = 0.2f;

    // Create outputs for each delay line
    std::vector<std::vector<float>> outputs(numDelayLines, std::vector<float>(numSamples, 0.0f));
    std::vector<std::vector<float>> feedbackSignals(numDelayLines, std::vector<float>(numSamples, 0.0f));

    // Create a buffer to store the reverb output for each channel
    std::vector<std::vector<float>> channelOutputs(numChannels, std::vector<float>(numSamples, 0.0f));

    // Process each sample
    for (int sample = 0; sample < numSamples; ++sample) {
        // Process each channel separately
        for (int ch = 0; ch < numChannels; ++ch) {
            // Get the input sample for this channel
            float inputSample = buffer.getSample(ch, sample);

            // Apply a safety check to input
            inputSample = juce::jlimit(-1.0f, 1.0f, inputSample);

            // Process through delay lines with feedback
            for (int i = 0; i < numDelayLines; ++i) {
                // Use modulo to distribute channels across delay lines if more channels than delay lines
                int delayIndex = (i + ch) % numDelayLines;

                // Apply feedback from previous frame if available - with safety limiter
                float feedbackSample = (sample > 0) ?
                    juce::jlimit(-1.0f, 1.0f, feedbackSignals[delayIndex][sample - 1]) : 0.0f;

                // Mix input with feedback for each delay line
                float delayInput = inputSample + decayGain *
                    lpfFilters[delayIndex].process(feedbackSample, lpfCutoff);

                // Safety limiter again
                delayInput = juce::jlimit(-1.0f, 1.0f, delayInput);

                // Apply diffusion with cascaded all-pass filters
                float diffusedInput = diffusionFilters[delayIndex].process(delayInput, diffusionCoeff);

                // Process through delay
                outputs[delayIndex][sample] = delayLines[delayIndex]->processSample(diffusedInput);
            }
        }

        // The rest of your code remains similar but with safety limiters
        std::array<float, numDelayLines> sampleVec;
        for (int i = 0; i < numDelayLines; ++i) {
            sampleVec[i] = juce::jlimit(-1.0f, 1.0f, outputs[i][sample]);
        }

        std::array<float, numDelayLines> mixedSample = { 0.0f, 0.0f, 0.0f, 0.0f };
        for (int row = 0; row < numDelayLines; ++row) {
            for (int col = 0; col < numDelayLines; ++col) {
                mixedSample[row] += hadamardMatrix[row][col] * sampleVec[col];
            }
            // Apply safety limiter and store the feedback signal
            feedbackSignals[row][sample] = juce::jlimit(-1.0f, 1.0f, mixedSample[row]);

            // Distribute reverb output back to each audio channel
            for (int ch = 0; ch < numChannels; ++ch) {
                channelOutputs[ch][sample] += mixedSample[(row + ch) % numDelayLines] / numDelayLines;
            }
        }
    }
    return channelOutputs;
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
>>>>>>> Stashed changes
