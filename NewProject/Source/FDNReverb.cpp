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

		// Modulated diffusers with different prime sizes
		int modSize = allPassValues[i] * 1.23f;
		if (modSize % 2 == 0) modSize++;
		modulatedDiffusers.push_back(ModulatedAllPassFilter(modSize));

		// Post-diffusion stage
        modSize = allPassValues[i] * 1.5f;
        if (modSize % 2 == 0) modSize++;
		postDiffusers.push_back(AllPassFilter(modSize));

		lpfFilters.push_back(BiquadFilter());
		hpfFilters.push_back(BiquadFilter());
		dcBlockers.push_back(DCBlocker());
	}

	// Different modulation rates for each diffuser
	for (int i = 0; i < numDelayLines; ++i) {
		float modRate = 0.1f + (0.3f * i / numDelayLines);
        float modDepth = 0.05f + (0.15f * i / numDelayLines);
		modulatedDiffusers[i].setModulation(modDepth, modRate);
	}

	// Early reflections
	earlyReflections = {
		{ 450,  0.65f },
		{ 850,  0.57f },
		{ 1250, 0.49f },
		{ 1800, 0.40f },
		{ 2500, 0.32f },
		{ 3200, 0.24f },
		{ 4000, 0.18f },
		{ 4800, 0.15f },
		{ 5400, 0.12f },
		{ 6000, 0.10f },
		{ 6500, 0.08f },
		{ 7000, 0.07f },
		{ 7500, 0.06f },
		{ 8000, 0.05f },
		{ 8500, 0.04f },
		{ 9000, 0.03f }
	};

	// Initialize ER buffer
	erBufferSize = 10000;
	erBuffer.resize(erBufferSize, 0.0f);

	// Additional diffusers for the early reflections
	erDiffusion1 = AllPassFilter();
	erDiffusion2 = AllPassFilter();

	predelayBuffer = PredelayLine(96000);
}

FDNReverb::~FDNReverb() {
}

void FDNReverb::prepare(double newSampleRate) {
	sampleRate = newSampleRate;

	// Consistent reverb time across different sample rates
	double sampleRateRatio = sampleRate / 44100.0;

	if (std::abs(sampleRateRatio - 1.0) > 0.01) {
		delayLines.clear();
		for (int i = 0; i < numDelayLines; ++i) {
			int scaledDelay = static_cast<int>(primeDelays[i] * sampleRateRatio);
			if (scaledDelay < 1) scaledDelay = 1;
			delayLines.push_back(std::make_unique<CustomDelayLine>(scaledDelay));
		}
	}

	// Reset Biquad Filters
	for (int i = 0; i < numDelayLines; ++i) {
		lpfFilters[i].setLowpass(5000.0f, 0.7071f, static_cast<float>(sampleRate));
		lpfFilters[i].z1 = 0.0f;
		lpfFilters[i].z2 = 0.0f;

		hpfFilters[i].setHighpass(120.0f, 0.7071f, static_cast<float>(sampleRate));
		hpfFilters[i].z1 = 0.0f;
		hpfFilters[i].z2 = 0.0f;
	}

	// Reset diffusion filters
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

	// Reset modulated filters
	for (auto& filter : modulatedDiffusers) {
		filter.clear();
	}

	for (auto& filter : postDiffusers) {
		filter.clear();
	}

	// Resize and clear ER buffer
	erBufferSize = static_cast<int>(10000 * sampleRateRatio);
	erBuffer.resize(erBufferSize, 0.0f);
	erWriteIndex = 0;
}

std::vector<std::vector<float>> FDNReverb::process(juce::AudioBuffer<float>& buffer,
    double predelay,
    double decay,
    double diffusion,
    double hpCutoff,
    double lpCutoff)
{
    const int numChannels = buffer.getNumChannels();
    const int numSamples = buffer.getNumSamples();

    float decayGain = juce::jlimit(0.0f, 0.98f, static_cast<float>(decay));
    float decayVariations[numDelayLines] = {
        1.0f, 0.998f, 0.997f, 0.999f, 0.996f, 0.998f, 0.997f, 0.999f,
        0.995f, 0.998f, 0.996f, 0.999f, 0.997f, 0.995f, 0.998f, 0.996f
    };
    float diffusionCoeff = juce::jlimit(0.0f, 0.9f, static_cast<float>(diffusion));

    int predelaySamples = static_cast<int>(predelay * sampleRate / 1000.0);

    constexpr float butterworthQ = 0.7071f;

    for (int i = 0; i < numDelayLines; ++i) {
        // Add slight variation to cutoffs per delay line for more natural sound
        float hpVariation = 0.95f + 0.1f * (static_cast<float>(i) / numDelayLines);
        float lpVariation = 0.97f + 0.06f * (static_cast<float>(i) / numDelayLines);

        hpfFilters[i].setHighpass(
            static_cast<float>(hpCutoff) * hpVariation,
            butterworthQ,
            static_cast<float>(sampleRate)
        );

        lpfFilters[i].setLowpass(
            static_cast<float>(lpCutoff) * lpVariation,
            butterworthQ,
            static_cast<float>(sampleRate)
        );
    }

    std::vector<std::vector<float>> outputs(numDelayLines, std::vector<float>(numSamples, 0.0f));
    std::vector<std::vector<float>> feedbackSignals(numDelayLines, std::vector<float>(numSamples, 0.0f));
    std::vector<std::vector<float>> channelOutputs(numChannels, std::vector<float>(numSamples, 0.0f));

    // Direct signal mix
    for (int ch = 0; ch < numChannels; ++ch)
    {
        for (int sample = 0; sample < numSamples; ++sample)
        {
            channelOutputs[ch][sample] = buffer.getSample(ch, sample) * 0.20f;
        }
    }

    // Early reflections
    for (int sample = 0; sample < numSamples; ++sample)
    {
        float monoInput = 0.0f;
        for (int ch = 0; ch < numChannels; ++ch)
            monoInput += buffer.getSample(ch, sample);
        monoInput /= numChannels;

        erBuffer[erWriteIndex] = monoInput;

        float erOutput = 0.0f;
        for (int i = 0; i < 8; i++)
        {
            const auto& er = earlyReflections[i];
            int readPos = erWriteIndex - er.delaySamples;
            if (readPos < 0)
                readPos += erBufferSize;
            erOutput += erBuffer[readPos] * er.gain;
        }

        erOutput = erDiffusion1.process(erOutput, 0.2f);

        erWriteIndex = (erWriteIndex + 1) % erBufferSize;

        for (int ch = 0; ch < numChannels; ++ch)
            channelOutputs[ch][sample] += erOutput * 0.80f;
    }

    for (int sample = 0; sample < numSamples; ++sample)
    {
        std::array<float, numDelayLines> inputSignals = { 0.0f };
        for (int ch = 0; ch < std::min(numChannels, numDelayLines); ++ch)
        {
            float inputSample = buffer.getSample(ch, sample);
            inputSample = dcBlockers[ch].process(inputSample);
            // Apply denormal prevention and then predelay
            inputSignals[ch] = predelayBuffer.process(denormalPrevention(inputSample), predelaySamples);
        }

        std::array<float, numDelayLines> mixedInputs = { 0.0f };
        for (int i = 0; i < numDelayLines; ++i)
        {
            for (int j = 0; j < numDelayLines; ++j)
            {
                mixedInputs[i] += hadamardMatrix[i][j] * inputSignals[j];
            }
        }

        // Start of feedback loop
        for (int i = 0; i < numDelayLines; ++i)
        {
            float prevFeedback = (sample > 0) ? feedbackSignals[i][sample - 1] * 0.95f : 0.0f;

            // Invert polarity to increase complexity
            bool invertInput = ((i & 0x1) != 0);
            float delayInput = (invertInput ? -1.0f : 1.0f) * mixedInputs[i] + prevFeedback;
            delayInput = lpfFilters[i].processBiquad(delayInput);
            outputs[i][sample] = delayLines[i]->processSample(delayInput);
        }

        std::array<float, numDelayLines> householderMixed = { 0.0f };
        for (int i = 0; i < numDelayLines; ++i)
        {
            for (int j = 0; j < numDelayLines; ++j)
                householderMixed[i] += householderMatrix[i][j] * outputs[j][sample];
        }

        // Noise gating with high-pass filtering and post diffusion
        for (int i = 0; i < numDelayLines; ++i)
        {
            float signal = dcBlockers[i].process(householderMixed[i]);

            // HPF
            signal = hpfFilters[i].processBiquad(signal);

            signal = diffusionFilters[i].process(signal, 0.4f + (diffusionCoeff * 0.1f));

			// Modulated diffusion with amplitude-sensitive depth
            if (diffusionCoeff > 0.5f) {
                float adaptiveDepth = (0.01f + (diffusionCoeff * 0.05f)) * std::min(1.0f, std::abs(signal) * 1.1f);

                // Apply modulated diffusion with amplitude-sensitive depth
                signal = modulatedDiffusers[i].process(signal, adaptiveDepth, static_cast<float>(sampleRate));
            }

            float postDiffCoeff = 0.05f + (diffusionCoeff * 0.1f);
            signal = postDiffusers[i].process(signal, postDiffCoeff);

            float lineDecay = decayGain * decayVariations[i];

            // Denormal prevention
            signal = denormalPrevention(signal);

            // Soft Limiting
            signal = softLimit(signal);

            // Higher noise gate threshold
            if (std::abs(signal) < 1e-4f)
                signal = 0.0f;

            // More progressive noise reduction with additional soft threshold
            if (std::abs(signal) < 5e-4f) {
                signal *= std::pow(std::abs(signal) / 5e-4f, 1.5f);
            }

            if (sample > 0) {
                float prevSample = feedbackSignals[i][sample - 1] / lineDecay;
                signal = prevSample * 0.4f + signal * 0.6f;
            }

            signal = std::tanh(signal * 0.9f) / 0.9f;

            // Reduce decay for low frequencies
            if (i < 4) {
                lineDecay *= 0.94f;
            }

            feedbackSignals[i][sample] = signal * lineDecay;
        }

        // Mix late reverb outputs for each channel
        for (int ch = 0; ch < numChannels; ++ch)
        {
            float lateSum = 0.0f;
            for (int i = 0; i < numDelayLines; i += 2)
            {
                float outputGain = 1.0f / (numDelayLines / 2);
                lateSum += feedbackSignals[(i + ch) % numDelayLines][sample] * outputGain;
            }
            channelOutputs[ch][sample] += lateSum;
            channelOutputs[ch][sample] = softLimit(channelOutputs[ch][sample]);
        }
    }

    return channelOutputs;
}

