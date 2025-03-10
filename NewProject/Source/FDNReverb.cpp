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
    for (int i = 0; i < numDelayLines; ++i) {
        delayLines.push_back(std::make_unique<CustomDelayLine>(primeDelays[i]));
        diffusionFilters.push_back(AllPassFilter());
        lpfFilters.push_back(BiquadFilter());
        lfos.push_back(LFO());
    }

    // Early reflection for a realistic room sound
    earlyReflections = {
        { 450,  0.50f },
        { 850,  0.40f },
        { 1250, 0.30f },
        { 1800, 0.25f },
        { 2500, 0.20f },
        { 3200, 0.15f }
    };

    // Initialize ER buffer (enough for longest reflection)
    erBufferSize = 4000;  // ~90ms at 44.1kHz
    erBuffer.resize(erBufferSize, 0.0f);

    // Additional diffusers for the early reflections
    erDiffusion1 = AllPassFilter();
    erDiffusion2 = AllPassFilter();

    predelayBuffer = PredelayLine(96000);  // Maximum 2 seconds at 48kHz
}

FDNReverb::~FDNReverb() {
}

void FDNReverb::prepare(double newSampleRate) {
    sampleRate = newSampleRate;

    // Maintain consistent reverb time across different sample rates
    double sampleRateRatio = sampleRate / 44100.0;

    // Only recreate delay lines if sample rate has changed significantly
    if (std::abs(sampleRateRatio - 1.0) > 0.01) {

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

    // Scale early reflection times for sample rate
    for (auto& er : earlyReflections) {
        er.delaySamples = static_cast<int>(er.delaySamples * sampleRateRatio);
    }

    // Resize and clear ER buffer
    erBufferSize = static_cast<int>(4000 * sampleRateRatio);
    erBuffer.resize(erBufferSize, 0.0f);
    erWriteIndex = 0;
}

std::vector<std::vector<float>> FDNReverb::process(juce::AudioBuffer<float>& buffer,
    double predelay,
    double decay,
    double diffusion)
{
    const int numChannels = buffer.getNumChannels();
    const int numSamples = buffer.getNumSamples();

    // Smoother gain decay
    float decayGain = juce::jlimit(0.0f, 0.992f, static_cast<float>(decay));
    float decayVariations[numDelayLines] = { 1.0f, 0.99f, 0.995f, 0.985f, 0.992f, 0.988f, 0.997f, 0.982f };
    float diffusionCoeff = juce::jlimit(0.0f, 0.7f, static_cast<float>(diffusion));

    int predelaySamples = static_cast<int>(predelay * sampleRate / 1000.0);

    float lpfCutoff = 0.2f;

    std::vector<std::vector<float>> outputs(numDelayLines, std::vector<float>(numSamples, 0.0f));
    std::vector<std::vector<float>> feedbackSignals(numDelayLines, std::vector<float>(numSamples, 0.0f));
    std::vector<std::vector<float>> channelOutputs(numChannels, std::vector<float>(numSamples, 0.0f));

    // Process early reflections first
    for (int sample = 0; sample < numSamples; ++sample) {
        // Mix down input to mono for early reflections
        float monoInput = 0.0f;
        for (int ch = 0; ch < numChannels; ++ch) {
            monoInput += buffer.getSample(ch, sample);
        }
        monoInput /= numChannels;

        // Write to circular buffer
        erBuffer[erWriteIndex] = monoInput;

        // Generate early reflection output for this sample
        float erOutput = 0.0f;

        for (const auto& er : earlyReflections) {
            // Calculate read position with wraparound
            int readPos = erWriteIndex - er.delaySamples;
            if (readPos < 0) readPos += erBufferSize;

            // Add this reflection
            erOutput += erBuffer[readPos] * er.gain;

            // Apply diffusion to early reflections
            erOutput = erDiffusion1.process(erOutput, 0.25f);
            erOutput = erDiffusion2.process(erOutput, 0.15f);
        }

        // Advance write position
        erWriteIndex = (erWriteIndex + 1) % erBufferSize;

        // Add early reflections to output channels
        for (int ch = 0; ch < numChannels; ++ch) {
            channelOutputs[ch][sample] += softLimit(erOutput);
        }
    }

    for (int sample = 0; sample < numSamples; ++sample)
    {
        // Generate LFO values for this sample
        float lfoValues[numDelayLines];
        for (int i = 0; i < numDelayLines; ++i) {
            // Different rates for each delay line
            float lfoFreq = 0.1f + i * 0.03f; // 0.1 Hz to 0.19 Hz
            lfoValues[i] = lfos[i].process(lfoFreq, sampleRate);
        }

        // 1) Delay lines
        for (int ch = 0; ch < numChannels; ++ch)
        {
            float inputSample = juce::jlimit(-1.0f, 1.0f, buffer.getSample(ch, sample));

            // Apply predelay to the input signal
            float delayedInput = predelayBuffer.process(inputSample, predelaySamples);

            for (int i = 0; i < numDelayLines; ++i)
            {
                float prevFeedback = (sample > 0)
                    ? feedbackSignals[i][sample - 1]
                    : 0.0f;

                // Sum predelayed input + previous feedback
                float delayInput = delayedInput + prevFeedback;

                // Apply very subtle modulation to the feedback signal
                delayInput *= (1.0f + (lfoValues[i] - 0.5f) * 0.01f); // Very subtle ±0.5% modulation

                // Store delayed output:
                outputs[i][sample] = delayLines[i]->processSample(
                    softLimit(delayInput));
            }
        }


        // Rest of the processing remains the same
        // ...

        // Gather delay output into an array:
        std::array<float, numDelayLines> sampleVec;
        for (int i = 0; i < numDelayLines; ++i)
            sampleVec[i] = outputs[i][sample];

        // 2) Apply Hadamard matrix:
        std::array<float, numDelayLines> matrixedSample{ 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f };
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
            matrixedSample[i] = diffused; // Store back for feedback matrix
        }
        
        // Apply Hadamard feedback matrix (cross-channel feedback mixing)
        std::array<float, numDelayLines> householderMixed = { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f };
        for (int i = 0; i < numDelayLines; ++i) {
            for (int j = 0; j < numDelayLines; ++j) {
                householderMixed[i] += householderMatrix[i][j] * matrixedSample[j];
            }
        }

        // Apply LPF and decay gain
        for (int i = 0; i < numDelayLines; ++i) {
            float filtered = lpfFilters[i].process(householderMixed[i], lpfCutoff);

            // Apply slightly different decay for each line
            float lineDecay = decayGain * decayVariations[i];

            // Apply a soft-knee compression to the decay to preserve quieter reverb tails
            float absLevel = std::abs(filtered);
            if (absLevel > 0.5f) {
                // Compress louder parts slightly more
                filtered *= (0.5f + (absLevel - 0.5f) * 0.8f) / absLevel;
            }

            float feedbackOut = softLimit(filtered * lineDecay);
            feedbackSignals[i][sample] = feedbackOut;
        }

        // Send to channel outputs
        for (int ch = 0; ch < numChannels; ++ch)
        {
            for (int i = 0; i < numDelayLines; ++i)
            {
                // Apply soft limiting and add to output
                channelOutputs[ch][sample] += softLimit(feedbackSignals[(i + ch) % numDelayLines][sample] / (numDelayLines * 0.8f));
            }
        }
    }
    return channelOutputs;
}