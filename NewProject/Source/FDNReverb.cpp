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
		hpfFilters.push_back(BiquadFilter());
		dcBlockers.push_back(DCBlocker());
	}

	// Early reflection for a realistic room sound
	earlyReflections = {
		{ 450,  0.65f },
		{ 850,  0.57f },
		{ 1250, 0.49f },
		{ 1800, 0.40f },
		{ 2500, 0.32f },
		{ 3200, 0.24f },
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
	for (int i = 0; i < numDelayLines; ++i) {
		lpfFilters[i].setLowpass(5000.0f, 0.7071f, static_cast<float>(sampleRate));
		lpfFilters[i].z1 = 0.0f;
		lpfFilters[i].z2 = 0.0f;

		// Initialize high-pass filters with Butterworth response
		hpfFilters[i].setHighpass(120.0f, 0.7071f, static_cast<float>(sampleRate));
		hpfFilters[i].z1 = 0.0f;
		hpfFilters[i].z2 = 0.0f;
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
	float decayVariations[numDelayLines] = { 1.0f, 0.998f, 0.997f, 0.999f, 0.996f, 0.998f, 0.997f, 0.999f };
	float diffusionCoeff = juce::jlimit(0.0f, 0.9f, static_cast<float>(diffusion));

	int predelaySamples = static_cast<int>(predelay * sampleRate / 1000.0);

	std::vector<std::vector<float>> outputs(numDelayLines, std::vector<float>(numSamples, 0.0f));
	std::vector<std::vector<float>> feedbackSignals(numDelayLines, std::vector<float>(numSamples, 0.0f));
	std::vector<std::vector<float>> channelOutputs(numChannels, std::vector<float>(numSamples, 0.0f));

	// Direct signal mix (moderate level)
	for (int ch = 0; ch < numChannels; ++ch)
	{
		for (int sample = 0; sample < numSamples; ++sample)
		{
			channelOutputs[ch][sample] = buffer.getSample(ch, sample) * 0.20f;
		}
	}

	// Process early reflections
	for (int sample = 0; sample < numSamples; ++sample)
	{
		float monoInput = 0.0f;
		for (int ch = 0; ch < numChannels; ++ch)
			monoInput += buffer.getSample(ch, sample);
		monoInput /= numChannels;

		erBuffer[erWriteIndex] = monoInput;

		float erOutput = 0.0f;
		for (int i = 0; i < 4; i++)  // Using first 4 reflections
		{
			const auto& er = earlyReflections[i];
			int readPos = erWriteIndex - er.delaySamples;
			if (readPos < 0)
				readPos += erBufferSize;
			erOutput += erBuffer[readPos] * er.gain;
		}

		erOutput = erDiffusion1.process(erOutput, 0.2f);

		erWriteIndex = (erWriteIndex + 1) % erBufferSize;

		// Increase early reflection contribution
		for (int ch = 0; ch < numChannels; ++ch)
			channelOutputs[ch][sample] += erOutput * 0.80f;
	}

	// Dynamic low-pass cutoff adjustment with Butterworth response
	float inputCutoff = 2900.0f + (1.0f - decayGain) * 2000.0f;
	constexpr float butterworthQ = 0.7071f; // For maximally flat frequency response

	for (int i = 0; i < numDelayLines; ++i) {
		// Add slight variation to cutoff per delay line for more natural sound
		float lineCutoff = inputCutoff * (0.97f + 0.06f * (static_cast<float>(i) / numDelayLines));
		lpfFilters[i].setLowpass(lineCutoff, butterworthQ, static_cast<float>(sampleRate));
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

		// FEEDBACK LOOP
		for (int i = 0; i < numDelayLines; ++i)
		{
			float prevFeedback = (sample > 0) ? feedbackSignals[i][sample - 1] * 0.95f : 0.0f;


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

		// Enhanced noise gating with high-pass filtering
		for (int i = 0; i < numDelayLines; ++i)
		{
			float signal = dcBlockers[i].process(householderMixed[i]);

			// Use the BiquadFilter for high-pass filtering
			signal = hpfFilters[i].processBiquad(signal);

			// Continue with standard diffusion
			signal = diffusionFilters[i].process(signal, 0.4f + (diffusionCoeff * 0.1f));
			float lineDecay = decayGain * decayVariations[i];

			// Apply denormal prevention and soft limit
			signal = denormalPrevention(signal);
			// More aggressive limiting
			if (std::abs(signal) > 0.9f)
				signal *= 0.9f / std::abs(signal);

			// Higher noise gate threshold
			if (std::abs(signal) < 1e-4f)  // Higher threshold
				signal = 0.0f;

			// More progressive noise reduction with additional soft threshold
			if (std::abs(signal) < 5e-4f) {
				signal *= std::pow(std::abs(signal) / 5e-4f, 1.5f);  // Progressive reduction
			}

			// Extra smooth between consecutive samples
			if (sample > 0) {
				float prevSample = feedbackSignals[i][sample - 1] / lineDecay;
				signal = prevSample * 0.4f + signal * 0.6f;
			}

			// Apply more aggressive soft saturation curve
			signal = std::tanh(signal * 0.9f) / 0.9f;

			// Reduce decay for low frequencies
			if (i < 2) {  // First two delay lines tend to carry more low frequencies
				lineDecay *= 0.94f;  // Extra decay reduction
			}

			feedbackSignals[i][sample] = signal * lineDecay;
		}

		// Mix late reverb outputs for each channel
		for (int ch = 0; ch < numChannels; ++ch)
		{
			float lateSum = 0.0f;
			for (int i = 0; i < numDelayLines; i += 2)
			{
				float outputGain = 1.2f / (numDelayLines / 2);
				lateSum += feedbackSignals[(i + ch) % numDelayLines][sample] * outputGain;
			}
			channelOutputs[ch][sample] += lateSum;
		}
	}

	return channelOutputs;
}


