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
        { 450,  0.55f },
        { 850,  0.45f },
        { 1250, 0.38f },
        { 1800, 0.32f },
        { 2500, 0.28f },
        { 3200, 0.22f },
        { 4000, 0.18f },
        { 4800, 0.15f }
    };

    // Initialize ER buffer (enough for longest reflection)
    erBufferSize = 5000;  // ~90ms at 44.1kHz
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

    // Reset diffusion filters using the clear method
    for (auto& filter : diffusionFilters) {
        filter.clear();
    }

    // Reset DC Blockers
    for (auto& blocker : dcBlockers) {
        blocker.reset();
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

    // Simplify parameter handling
    float decayGain = juce::jlimit(0.0f, 0.98f, static_cast<float>(decay));
    // Use fewer variations with less extreme differences
    float decayVariations[numDelayLines] = { 1.0f, 0.998f, 0.997f, 0.999f, 0.996f, 0.998f, 0.997f, 0.999f };
    float diffusionCoeff = juce::jlimit(0.0f, 0.7f, static_cast<float>(diffusion));

    int predelaySamples = static_cast<int>(predelay * sampleRate / 1000.0);

    std::vector<std::vector<float>> outputs(numDelayLines, std::vector<float>(numSamples, 0.0f));
    std::vector<std::vector<float>> feedbackSignals(numDelayLines, std::vector<float>(numSamples, 0.0f));
    std::vector<std::vector<float>> channelOutputs(numChannels, std::vector<float>(numSamples, 0.0f));

    // Add some direct signal for immediate presence
    for (int ch = 0; ch < numChannels; ++ch) {
        for (int sample = 0; sample < numSamples; ++sample) {
            // Add 15% direct signal
            channelOutputs[ch][sample] = buffer.getSample(ch, sample) * 0.15f;
        }
    }

    // Process early reflections with simplified approach
    for (int sample = 0; sample < numSamples; ++sample) {
        float monoInput = 0.0f;
        for (int ch = 0; ch < numChannels; ++ch) {
            monoInput += buffer.getSample(ch, sample);
        }
        monoInput /= numChannels;

        // Write to circular buffer
        erBuffer[erWriteIndex] = monoInput;

        // Sum early reflection contributions - simplify to just a few key reflections
        float erOutput = 0.0f;
        for (int i = 0; i < 4; i++) { // Use only the first 4 reflections for clarity
            const auto& er = earlyReflections[i];
            int readPos = erWriteIndex - er.delaySamples;
            if (readPos < 0)
                readPos += erBufferSize;
            erOutput += erBuffer[readPos] * er.gain;
        }

        // Apply minimal diffusion - just one stage with moderate setting
        erOutput = erDiffusion1.process(erOutput, 0.2f);

        // Advance write position
        erWriteIndex = (erWriteIndex + 1) % erBufferSize;

        for (int ch = 0; ch < numChannels; ++ch) {
            channelOutputs[ch][sample] += erOutput * 0.6f;
        }
    }

    // Set consistent filter cutoff for cleaner sound
    float inputCutoff = 3000.0f + (1.0f - decayGain) * 2000.0f;
    for (int i = 0; i < numDelayLines; ++i) {
        lpfFilters[i].setLowpass(inputCutoff, 0.6f, static_cast<float>(sampleRate));
    }

    for (int sample = 0; sample < numSamples; ++sample)
    {
        std::array<float, numDelayLines> inputSignals = { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f };

        for (int ch = 0; ch < std::min(numChannels, numDelayLines); ++ch) {
            float inputSample = buffer.getSample(ch, sample);
            inputSample = dcBlockers[ch].process(inputSample);
            inputSignals[ch] = predelayBuffer.process(inputSample, predelaySamples);
        }

        std::array<float, numDelayLines> mixedInputs = { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f };
        for (int i = 0; i < numDelayLines; ++i) {
            for (int j = 0; j < numDelayLines; ++j) {
                mixedInputs[i] += hadamardMatrix[i][j] * inputSignals[j];
            }
        }

        for (int i = 0; i < numDelayLines; ++i)
        {
            // Simplify feedback mixing
            float prevFeedback = (sample > 0) ? feedbackSignals[i][sample - 1] * 0.95f : 0.0f;

            // Simpler polarity pattern
            bool invertInput = (i & 0x1) != 0;   // Alternate polarity

            // Simplified mixing
            float delayInput = (invertInput ? -1.0f : 1.0f) * mixedInputs[i] + prevFeedback;

            // Apply filter and delay
            delayInput = lpfFilters[i].processBiquad(delayInput);
            outputs[i][sample] = delayLines[i]->processSample(delayInput);
        }

        std::array<float, numDelayLines> householderMixed = { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f };
        for (int i = 0; i < numDelayLines; ++i) {
            for (int j = 0; j < numDelayLines; ++j) {
                householderMixed[i] += householderMatrix[i][j] * outputs[j][sample];
            }
        }

        for (int i = 0; i < numDelayLines; ++i) {
            float signal = dcBlockers[i].process(householderMixed[i]);

            // Simplified diffusion - use only one stage with consistent settings
            signal = diffusionFilters[i].process(signal, 0.3f + (diffusionCoeff * 0.1f));

            // Apply decay
            float lineDecay = decayGain * decayVariations[i];

            // Simple limiter to avoid excessive peaks
            if (std::abs(signal) > 0.9f)
                signal *= 0.9f / std::abs(signal);

            feedbackSignals[i][sample] = signal * lineDecay;
        }

        for (int ch = 0; ch < numChannels; ++ch) {
            float lateSum = 0.0f;
            // Use fewer lines for cleaner sound
            for (int i = 0; i < numDelayLines; i += 2) {
                float outputGain = 1.2f / (numDelayLines / 2);
                lateSum += feedbackSignals[(i + ch) % numDelayLines][sample] * outputGain;
            }
            channelOutputs[ch][sample] += lateSum;
        }
    }

    return channelOutputs;
}

