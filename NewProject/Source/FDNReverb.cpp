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

    float decayGain = juce::jlimit(0.0f, 0.98f, static_cast<float>(decay));
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
            // * 0.8 for more prominent late reverb
            channelOutputs[ch][sample] += softLimit(erOutput * 0.8f);
        }
    }

    for (int sample = 0; sample < numSamples; ++sample)
    {
        // 1a) First prepare input signals
        std::array<float, numDelayLines> inputSignals = { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f };

        for (int ch = 0; ch < std::min(numChannels, numDelayLines); ++ch) {
            // Less conservative input limiting to allow more signal through
            float inputSample = juce::jlimit(-0.85f, 0.85f, buffer.getSample(ch, sample));

            // DC Block to reduce low frequency buildup
            inputSample = dcBlockers[ch].process(inputSample);

            // Predelay
            inputSignals[ch] = predelayBuffer.process(inputSample, predelaySamples);
        }

        // 1b) Mixing matrix -> HADAMARD
        std::array<float, numDelayLines> mixedInputs = { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f };
        for (int i = 0; i < numDelayLines; ++i) {
            for (int j = 0; j < numDelayLines; ++j) {
                mixedInputs[i] += hadamardMatrix[i][j] * inputSignals[j];
            }
        }   

        // 1c) Delay lines with feedback - elegant polarity pattern
        for (int i = 0; i < numDelayLines; ++i)
        {
            // Increase feedback to 98% for more sustain
            float prevFeedback = (sample > 0) ? feedbackSignals[i][sample - 1] * 0.98f : 0.0f;

            // Apply polarity inversion in an elegant alternating pattern
            // Create a binary pattern based on prime numbers to avoid regular patterns
            bool invertInput = (i & 0x5) != 0;  // Binary 101 pattern (bits 0 and 2)
            bool invertFeedback = (i & 0x3) != 0;  // Binary 011 pattern (bits 0 and 1)

            // Apply polarity based on pattern
            float delayInput = (invertInput ? -1.0f : 1.0f) * mixedInputs[i] +
                (invertFeedback ? -1.0f : 1.0f) * prevFeedback;

            // Use slightly higher cutoff for more reverb presence
            float inputCutoff = 2000.0f + (1.0f - decayGain) * 2500.0f;
            lpfFilters[i].setLowpass(inputCutoff, 0.5f, static_cast<float>(sampleRate));
            delayInput = lpfFilters[i].processBiquad(delayInput);

            // Store delayed output with limiting
            outputs[i][sample] = delayLines[i]->processSample(softLimit(delayInput));
        }


        // 2) Feedback Matrix -> Householder
        std::array<float, numDelayLines> householderMixed = { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f };
        for (int i = 0; i < numDelayLines; ++i) {
            for (int j = 0; j < numDelayLines; ++j) {
                householderMixed[i] += householderMatrix[i][j] * outputs[j][sample];
            }
        }

        // 3) Post-processing with slightly more diffusion
        for (int i = 0; i < numDelayLines; ++i) {
            // Apply DC blocking after the Householder matrix
            float signal = dcBlockers[i].process(denormalPrevention(householderMixed[i]));

            // Diffusion -> increased perceived density
            int stages = 1 + static_cast<int>(diffusionCoeff * 2.0f); // 2 Stages
            for (int s = 0; s < stages; s++) {
                signal = diffusionFilters[i].process(signal, 0.35f + (diffusionCoeff * 0.2f));
            }

            // Apply decay and compression
            float lineDecay = decayGain * decayVariations[i];
            float absLevel = std::abs(signal);
            if (absLevel > 0.3f) {
                signal *= (0.3f + (absLevel - 0.3f) * 0.7f) / absLevel;
            }

            feedbackSignals[i][sample] = softLimit(signal * lineDecay);
        }

        // 4) Mix to output channels
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