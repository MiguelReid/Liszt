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
        diffusionFilters.push_back(AllPassFilter(allPassValues[i]));
        lpfFilters.push_back(BiquadFilter());
        dcBlockers.push_back(DCBlocker());
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

    // Reset Biquad Filters
    for (auto& filter : lpfFilters) {
        filter.setLowpass(5000.0f, 0.7071f, static_cast<float>(sampleRate));
        filter.z1 = 0.0f;
        filter.z2 = 0.0f;
    }

    // Reset diffusion filters
    for (auto& filter : diffusionFilters) {
        // Clear all buffer content
        std::fill(filter.buffer.begin(), filter.buffer.end(), 0.0f);
        filter.writeIndex = 0;
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

        // 1a) First prepare input signals from all channels using Hadamard matrix
        std::array<float, numDelayLines> inputSignals = { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f };

        // Get channel inputs with predelay
        for (int ch = 0; ch < std::min(numChannels, numDelayLines); ++ch) {
            float inputSample = juce::jlimit(-1.0f, 1.0f, buffer.getSample(ch, sample));
            inputSignals[ch] = predelayBuffer.process(inputSample, predelaySamples);
        }

        // Mixing matrix -> HADAMARD
        std::array<float, numDelayLines> mixedInputs = { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f };
        for (int i = 0; i < numDelayLines; ++i) {
            for (int j = 0; j < numDelayLines; ++j) {
                mixedInputs[i] += hadamardMatrix[i][j] * inputSignals[j];
            }
        }

        // 1b) Delay lines with feedback
        for (int i = 0; i < numDelayLines; ++i)
        {
            float prevFeedback = (sample > 0) ? feedbackSignals[i][sample - 1] : 0.0f;

            // Sum mixed input + feedback
            float delayInput = mixedInputs[i] + prevFeedback;

            // Store delayed output
            outputs[i][sample] = delayLines[i]->processSample(softLimit(delayInput));
        }

        // 2) Get delay line outputs
        std::array<float, numDelayLines> delayOutputs;
        for (int i = 0; i < numDelayLines; ++i) {
            delayOutputs[i] = outputs[i][sample];
        }

        // 3) Process feedback using Householder matrix
        std::array<float, numDelayLines> householderMixed = { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f };
        for (int i = 0; i < numDelayLines; ++i) {
            for (int j = 0; j < numDelayLines; ++j) {
                householderMixed[i] += householderMatrix[i][j] * delayOutputs[j];
            }
        }

        // 4) Process each delay line with proper signal flow
        for (int i = 0; i < numDelayLines; ++i) {
            // Apply DC blocking to prevent static buildup
            float signal = dcBlockers[i].process(denormalPrevention(householderMixed[i]));

            // Apply diffusion for density
            int stages = 1 + static_cast<int>(diffusionCoeff * 3.0f); // 3 MAX Stages
            for (int s = 0; s < stages; s++) {
                signal = diffusionFilters[i].process(signal, 0.4f + (diffusionCoeff * 0.3f));
            }

            // Apply frequency-dependent filtering
            float cutoff = 3000.0f + (1.0f - decayGain) * 5000.0f;
            lpfFilters[i].setLowpass(cutoff, 0.7071f, static_cast<float>(sampleRate));
            signal = lpfFilters[i].processBiquad(signal);

            // Apply decay and compression
            float lineDecay = decayGain * decayVariations[i];
            float absLevel = std::abs(signal);
            if (absLevel > 0.4f) {
                signal *= (0.4f + (absLevel - 0.4f) * 0.8f) / absLevel;
            }

            feedbackSignals[i][sample] = softLimit(signal * lineDecay);
        }

        // 5) Mix to output channels
        for (int ch = 0; ch < numChannels; ++ch) {
            for (int i = 0; i < numDelayLines; ++i) {
                float outputGain = 1.5f / numDelayLines;
                channelOutputs[ch][sample] += softLimit(
                    feedbackSignals[(i + ch) % numDelayLines][sample] * outputGain);
            }
        }
    }

    return channelOutputs;
}
