/*
  ==============================================================================

    FDNReverb.cpp
    Created: 18 Feb 2025 10:24:35am
    Author:  mikey

  ==============================================================================
*/

#include "FDNReverb.h"

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

std::vector<std::vector<float>> FDNReverb::process(juce::AudioBuffer<float>& buffer,
    double predelay,
    double decay,
    double diffusion,
    double dryWet)
{
    const int numChannels = buffer.getNumChannels();
    const int numSamples = buffer.getNumSamples();

    float decayGain = juce::jlimit(0.0f, 0.95f, static_cast<float>(decay));
    float diffusionCoeff = juce::jlimit(0.0f, 0.5f, static_cast<float>(diffusion));
    float lpfCutoff = 0.2f;

    std::vector<std::vector<float>> outputs(numDelayLines, std::vector<float>(numSamples, 0.0f));
    std::vector<std::vector<float>> feedbackSignals(numDelayLines, std::vector<float>(numSamples, 0.0f));
    std::vector<std::vector<float>> channelOutputs(numChannels, std::vector<float>(numSamples, 0.0f));

    for (int sample = 0; sample < numSamples; ++sample)
    {
        // 1) Delay lines:
        for (int ch = 0; ch < numChannels; ++ch)
        {
            float inputSample = juce::jlimit(-1.0f, 1.0f, buffer.getSample(ch, sample));

            for (int i = 0; i < numDelayLines; ++i)
            {
                float prevFeedback = (sample > 0)
                    ? feedbackSignals[i][sample - 1]
                    : 0.0f;

                // Sum input + previous feedback (no decay or filter at this stage):
                float delayInput = inputSample + prevFeedback;

                // Store delayed output:
                outputs[i][sample] = delayLines[i]->processSample(
                    juce::jlimit(-1.0f, 1.0f, delayInput));
            }
        }

        // Gather delay output into an array:
        std::array<float, numDelayLines> sampleVec;
        for (int i = 0; i < numDelayLines; ++i)
            sampleVec[i] = outputs[i][sample];

        // 2) Apply Hadamard (or Velvet) matrix:
        std::array<float, numDelayLines> matrixedSample{ 0.0f, 0.0f, 0.0f, 0.0f };
        for (int row = 0; row < numDelayLines; ++row)
        {
            for (int col = 0; col < numDelayLines; ++col)
            {
                matrixedSample[row] += hadamardMatrix[row][col] * sampleVec[col];
            }
        }

        // 3) Diffusion in the feedback loop & 4) Low-pass last:
        for (int i = 0; i < numDelayLines; ++i)
        {
            float diffused = diffusionFilters[i].process(matrixedSample[i], diffusionCoeff);
            diffused = lpfFilters[i].process(diffused, lpfCutoff);

            // Apply decay and safety limit:
            float feedbackOut = juce::jlimit(-1.0f, 1.0f, diffused * decayGain);

            // Store for next iteration’s feedback:
            feedbackSignals[i][sample] = feedbackOut;
        }

        // Send to channel outputs:
        for (int ch = 0; ch < numChannels; ++ch)
        {
            for (int i = 0; i < numDelayLines; ++i)
            {
                // Distribute among channels:
                channelOutputs[ch][sample] += feedbackSignals[(i + ch) % numDelayLines][sample]
                    / numDelayLines;
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
