﻿/*
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
        lpfFilters.push_back(BiquadFilter());
        lfos.push_back(LFO()); // Initialize LFOs
    }
}

FDNReverb::~FDNReverb() {
}

void FDNReverb::prepare(double newSampleRate) {
    // Store the new sample rate
    sampleRate = newSampleRate;

    // Scale delay lengths based on the new sample rate
    // This maintains consistent reverb time across different sample rates
    double sampleRateRatio = sampleRate / 44100.0; // Assuming original design was for 44.1kHz

    // Only recreate delay lines if sample rate has changed significantly
    if (std::abs(sampleRateRatio - 1.0) > 0.01) {
        // Calculate scaled delay lengths
        const int primeDelays[4] = { 1031, 1327, 1523, 1871 };

        // Recreate delay lines with scaled lengths
        delayLines.clear();
        for (int i = 0; i < numDelayLines; ++i) {
            int scaledDelay = static_cast<int>(primeDelays[i] * sampleRateRatio);
            if (scaledDelay < 1) scaledDelay = 1; // Ensure minimum delay
            delayLines.push_back(std::make_unique<CustomDelayLine>(scaledDelay));
        }
    }

    // Reset LFO phases to avoid clicks when changing sample rate
    for (auto& lfo : lfos) {
        lfo.phase = 0.0f;
    }

    // Reset filter states to prevent artifacts
    for (auto& filter : lpfFilters) {
        filter.z1 = 0.0f;
        filter.z2 = 0.0f;
    }

    // Reset diffusion filters
    for (auto& filter : diffusionFilters) {
        filter.buffer1[0] = 0.0f;
        filter.buffer1[1] = 0.0f;
        filter.buffer2[0] = 0.0f;
        filter.buffer2[1] = 0.0f;
    }
}

std::vector<std::vector<float>> FDNReverb::process(juce::AudioBuffer<float>& buffer,
    double predelay,
    double decay,
    double diffusion)
{
    const int numChannels = buffer.getNumChannels();
    const int numSamples = buffer.getNumSamples();

    float decayGain = juce::jlimit(0.0f, 0.98f, static_cast<float>(decay));
    float diffusionCoeff = juce::jlimit(0.0f, 0.7f, static_cast<float>(diffusion));
    float lpfCutoff = 0.1f;

    std::vector<std::vector<float>> outputs(numDelayLines, std::vector<float>(numSamples, 0.0f));
    std::vector<std::vector<float>> feedbackSignals(numDelayLines, std::vector<float>(numSamples, 0.0f));
    std::vector<std::vector<float>> channelOutputs(numChannels, std::vector<float>(numSamples, 0.0f));

    for (int sample = 0; sample < numSamples; ++sample)
    {
        // Generate LFO values for this sample
        float lfoValues[numDelayLines];
        for (int i = 0; i < numDelayLines; ++i) {
            // Different rates for each delay line
            float lfoFreq = 0.1f + i * 0.03f; // 0.1 Hz to 0.19 Hz
            lfoValues[i] = lfos[i].process(lfoFreq, sampleRate);
        }

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

                // Apply very subtle modulation to the feedback signal
                // This will create a more natural sound without changing the basic algorithm
                delayInput *= (1.0f + (lfoValues[i] - 0.5f) * 0.01f); // Very subtle ±0.5% modulation

                // Store delayed output:
                outputs[i][sample] = delayLines[i]->processSample(
                    juce::jlimit(-1.0f, 1.0f, delayInput));
            }
        }

        // Rest of the processing remains the same
        // ...

        // Gather delay output into an array:
        std::array<float, numDelayLines> sampleVec;
        for (int i = 0; i < numDelayLines; ++i)
            sampleVec[i] = outputs[i][sample];

        // 2) Apply Hadamard matrix:
        std::array<float, numDelayLines> matrixedSample{ 0.0f, 0.0f, 0.0f, 0.0f };
        for (int row = 0; row < numDelayLines; ++row)
        {
            for (int col = 0; col < numDelayLines; ++col)
            {
                matrixedSample[row] += hadamardMatrix[row][col] * sampleVec[col];
            }
        }

        // 3) Apply diffusion, then hadamard matrix, then LPF in the feedback loop
        for (int i = 0; i < numDelayLines; ++i)
        {
            // Apply diffusion
            float diffused = diffusionFilters[i].process(matrixedSample[i], diffusionCoeff);
            matrixedSample[i] = diffused; // Store back for Velvet mixing
        }
        
        // Apply Hadamard feedback matrix (cross-channel feedback mixing)
        std::array<float, numDelayLines> hadamardMixed = { 0.0f, 0.0f, 0.0f, 0.0f };
        for (int i = 0; i < numDelayLines; ++i) {
            for (int j = 0; j < numDelayLines; ++j) {
                hadamardMixed[i] += hadamardMatrix[i][j] * matrixedSample[j];
            }
        }

        // Apply LPF and decay gain
        for (int i = 0; i < numDelayLines; ++i) {
            // Now using the fixed process method
            float filtered = lpfFilters[i].process(hadamardMixed[i], lpfCutoff);

            // Apply decay and safety limit:
            float feedbackOut = juce::jlimit(-1.0f, 1.0f, filtered * decayGain);

            // Store for next iteration's feedback:
            feedbackSignals[i][sample] = feedbackOut;
        }

        // Send to channel outputs
        for (int ch = 0; ch < numChannels; ++ch)
        {
            for (int i = 0; i < numDelayLines; ++i)
            {
                // Increase reverb contribution to outputs
                channelOutputs[ch][sample] += feedbackSignals[(i + ch) % numDelayLines][sample]
                    / (numDelayLines * 0.7f);
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
