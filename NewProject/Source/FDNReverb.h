/*
  ==============================================================================

    FDNReverb.h
    Created: 18 Feb 2025 10:24:35am
    Author:  mikey

  ==============================================================================
*/

#pragma once
#include <JuceHeader.h>
#include <vector>
#include <memory>
#include <random>
#include <iostream>
#include <array>

class CustomDelayLine {
public:
    CustomDelayLine(int delaySamples) : delaySamples(delaySamples) {
        buffer.resize(delaySamples, 0.0f);
    }

    float processSample(float inputSample) {
        float outputSample = buffer[writeIndex];
        buffer[writeIndex] = inputSample;
        writeIndex = (writeIndex + 1) % delaySamples;
        return outputSample;
    }

private:
    int delaySamples;
    std::vector<float> buffer;
    int writeIndex = 0;
};

class PredelayLine {
public:
    PredelayLine(int maxDelay) {
        buffer.resize(maxDelay, 0.0f);
    }

    float process(float input, int delaySamples) {
        delaySamples = juce::jlimit(0, static_cast<int>(buffer.size() - 1), delaySamples);

        float output = buffer[readIndex];
        buffer[writeIndex] = input;

        writeIndex = (writeIndex + 1) % buffer.size();
        readIndex = (writeIndex - delaySamples + buffer.size()) % buffer.size();

        return output;
    }

private:
    std::vector<float> buffer;
    int writeIndex = 0, readIndex = 0;
};


class FDNReverb
{
public:
    FDNReverb();
    ~FDNReverb();

    std::vector<std::vector<float>> FDNReverb::process(juce::AudioBuffer<float>& buffer, double predelay, double decay, double diffusion, double hpCutoff, double lpCutoff);
    void prepare(double newSampleRate);

private:
    // DelayLines
    std::vector<std::unique_ptr<CustomDelayLine>> delayLines;
    static constexpr int numDelayLines = 16;
    const int primeDelays[numDelayLines] = {
        83, 89, 97, 101, 103, 109, 113, 121,
        127, 131, 137, 139, 149, 151, 157, 163
    };

    PredelayLine predelayBuffer{ 96000 }; // Maximum 2 seconds at 48kHz 

    // Normalised Hadamard matrix
    std::vector<std::vector<float>> FDNReverb::hadamard(const std::vector<std::vector<float>>& delayOutputs);
    const std::array<std::array<float, numDelayLines>, numDelayLines> hadamardMatrix = { {
        {  0.25f,  0.25f,  0.25f,  0.25f,  0.25f,  0.25f,  0.25f,  0.25f,  0.25f,  0.25f,  0.25f,  0.25f,  0.25f,  0.25f,  0.25f,  0.25f },
        {  0.25f, -0.25f,  0.25f, -0.25f,  0.25f, -0.25f,  0.25f, -0.25f,  0.25f, -0.25f,  0.25f, -0.25f,  0.25f, -0.25f,  0.25f, -0.25f },
        {  0.25f,  0.25f, -0.25f, -0.25f,  0.25f,  0.25f, -0.25f, -0.25f,  0.25f,  0.25f, -0.25f, -0.25f,  0.25f,  0.25f, -0.25f, -0.25f },
        {  0.25f, -0.25f, -0.25f,  0.25f,  0.25f, -0.25f, -0.25f,  0.25f,  0.25f, -0.25f, -0.25f,  0.25f,  0.25f, -0.25f, -0.25f,  0.25f },
        {  0.25f,  0.25f,  0.25f,  0.25f, -0.25f, -0.25f, -0.25f, -0.25f,  0.25f,  0.25f,  0.25f,  0.25f, -0.25f, -0.25f, -0.25f, -0.25f },
        {  0.25f, -0.25f,  0.25f, -0.25f, -0.25f,  0.25f, -0.25f,  0.25f,  0.25f, -0.25f,  0.25f, -0.25f, -0.25f,  0.25f, -0.25f,  0.25f },
        {  0.25f,  0.25f, -0.25f, -0.25f, -0.25f, -0.25f,  0.25f,  0.25f,  0.25f,  0.25f, -0.25f, -0.25f, -0.25f, -0.25f,  0.25f,  0.25f },
        {  0.25f, -0.25f, -0.25f,  0.25f, -0.25f,  0.25f,  0.25f, -0.25f,  0.25f, -0.25f, -0.25f,  0.25f, -0.25f,  0.25f,  0.25f, -0.25f },
        {  0.25f,  0.25f,  0.25f,  0.25f,  0.25f,  0.25f,  0.25f,  0.25f, -0.25f, -0.25f, -0.25f, -0.25f, -0.25f, -0.25f, -0.25f, -0.25f },
        {  0.25f, -0.25f,  0.25f, -0.25f,  0.25f, -0.25f,  0.25f, -0.25f, -0.25f,  0.25f, -0.25f,  0.25f, -0.25f,  0.25f, -0.25f,  0.25f },
        {  0.25f,  0.25f, -0.25f, -0.25f,  0.25f,  0.25f, -0.25f, -0.25f, -0.25f, -0.25f,  0.25f,  0.25f, -0.25f, -0.25f,  0.25f,  0.25f },
        {  0.25f, -0.25f, -0.25f,  0.25f,  0.25f, -0.25f, -0.25f,  0.25f, -0.25f,  0.25f,  0.25f, -0.25f, -0.25f,  0.25f,  0.25f, -0.25f },
        {  0.25f,  0.25f,  0.25f,  0.25f, -0.25f, -0.25f, -0.25f, -0.25f, -0.25f, -0.25f, -0.25f, -0.25f,  0.25f,  0.25f,  0.25f,  0.25f },
        {  0.25f, -0.25f,  0.25f, -0.25f, -0.25f,  0.25f, -0.25f,  0.25f, -0.25f,  0.25f, -0.25f,  0.25f,  0.25f, -0.25f,  0.25f, -0.25f },
        {  0.25f,  0.25f, -0.25f, -0.25f, -0.25f, -0.25f,  0.25f,  0.25f, -0.25f, -0.25f,  0.25f,  0.25f,  0.25f,  0.25f, -0.25f, -0.25f },
        {  0.25f, -0.25f, -0.25f,  0.25f, -0.25f,  0.25f,  0.25f, -0.25f, -0.25f,  0.25f,  0.25f, -0.25f,  0.25f, -0.25f, -0.25f,  0.25f }
    } };

	// Normalised Householder matrix
    const std::array<std::array<float, numDelayLines>, numDelayLines> householderMatrix = { {
        {  0.875f, -0.125f, -0.125f, -0.125f, -0.125f, -0.125f, -0.125f, -0.125f, -0.125f, -0.125f, -0.125f, -0.125f, -0.125f, -0.125f, -0.125f, -0.125f },
        { -0.125f,  0.875f, -0.125f, -0.125f, -0.125f, -0.125f, -0.125f, -0.125f, -0.125f, -0.125f, -0.125f, -0.125f, -0.125f, -0.125f, -0.125f, -0.125f },
        { -0.125f, -0.125f,  0.875f, -0.125f, -0.125f, -0.125f, -0.125f, -0.125f, -0.125f, -0.125f, -0.125f, -0.125f, -0.125f, -0.125f, -0.125f, -0.125f },
        { -0.125f, -0.125f, -0.125f,  0.875f, -0.125f, -0.125f, -0.125f, -0.125f, -0.125f, -0.125f, -0.125f, -0.125f, -0.125f, -0.125f, -0.125f, -0.125f },
        { -0.125f, -0.125f, -0.125f, -0.125f,  0.875f, -0.125f, -0.125f, -0.125f, -0.125f, -0.125f, -0.125f, -0.125f, -0.125f, -0.125f, -0.125f, -0.125f },
        { -0.125f, -0.125f, -0.125f, -0.125f, -0.125f,  0.875f, -0.125f, -0.125f, -0.125f, -0.125f, -0.125f, -0.125f, -0.125f, -0.125f, -0.125f, -0.125f },
        { -0.125f, -0.125f, -0.125f, -0.125f, -0.125f, -0.125f,  0.875f, -0.125f, -0.125f, -0.125f, -0.125f, -0.125f, -0.125f, -0.125f, -0.125f, -0.125f },
        { -0.125f, -0.125f, -0.125f, -0.125f, -0.125f, -0.125f, -0.125f,  0.875f, -0.125f, -0.125f, -0.125f, -0.125f, -0.125f, -0.125f, -0.125f, -0.125f },
        { -0.125f, -0.125f, -0.125f, -0.125f, -0.125f, -0.125f, -0.125f, -0.125f,  0.875f, -0.125f, -0.125f, -0.125f, -0.125f, -0.125f, -0.125f, -0.125f },
        { -0.125f, -0.125f, -0.125f, -0.125f, -0.125f, -0.125f, -0.125f, -0.125f, -0.125f,  0.875f, -0.125f, -0.125f, -0.125f, -0.125f, -0.125f, -0.125f },
        { -0.125f, -0.125f, -0.125f, -0.125f, -0.125f, -0.125f, -0.125f, -0.125f, -0.125f, -0.125f,  0.875f, -0.125f, -0.125f, -0.125f, -0.125f, -0.125f },
        { -0.125f, -0.125f, -0.125f, -0.125f, -0.125f, -0.125f, -0.125f, -0.125f, -0.125f, -0.125f, -0.125f,  0.875f, -0.125f, -0.125f, -0.125f, -0.125f },
        { -0.125f, -0.125f, -0.125f, -0.125f, -0.125f, -0.125f, -0.125f, -0.125f, -0.125f, -0.125f, -0.125f, -0.125f,  0.875f, -0.125f, -0.125f, -0.125f },
        { -0.125f, -0.125f, -0.125f, -0.125f, -0.125f, -0.125f, -0.125f, -0.125f, -0.125f, -0.125f, -0.125f, -0.125f, -0.125f,  0.875f, -0.125f, -0.125f },
        { -0.125f, -0.125f, -0.125f, -0.125f, -0.125f, -0.125f, -0.125f, -0.125f, -0.125f, -0.125f, -0.125f, -0.125f, -0.125f, -0.125f,  0.875f, -0.125f },
        { -0.125f, -0.125f, -0.125f, -0.125f, -0.125f, -0.125f, -0.125f, -0.125f, -0.125f, -0.125f, -0.125f, -0.125f, -0.125f, -0.125f, -0.125f,  0.875f }
    } };

    struct AllPassFilter {
        std::vector<float> buffer;
        int bufferSize = 0;
        int writeIndex = 0;
        // Use the last output for smoother transitions (and avoid clipping)
        float lastOutput = 0.0f;

        AllPassFilter(int size = 277) { 
            buffer.resize(size, 0.0f);
            bufferSize = size;
        }

        // Process the APFs
        float process(float input, float coeff) {
            coeff = juce::jlimit(-0.9f, 0.9f, coeff);

            int readIndex = (writeIndex - bufferSize + static_cast<int>(buffer.size())) % static_cast<int>(buffer.size());
            float delayedSample = buffer[readIndex];

            // Apply cascade of two first-order all-pass sections
            float temp = input + (coeff * delayedSample);
            buffer[writeIndex] = temp;
            writeIndex = (writeIndex + 1) % buffer.size();

            float output = delayedSample - (coeff * temp);

            // Smooth transitions to reduce THD
            output = 0.85f * output + 0.15f * lastOutput;
            lastOutput = output;

            // Soft saturation to reduce peaks which cause distortion
            if (std::abs(output) > 0.9f)
                output = std::tanh(output);

            return output;
        }

		// Cascaded diffusion with two stages
        float processMultiStage(float input, float coeff) {
            float stage1 = process(input, coeff);
            float stage2 = process(stage1, coeff * 0.85f);
            return stage2;
        }

        // Reset the filter states
        void clear() noexcept {
            std::fill(buffer.begin(), buffer.end(), 0.0f);
            writeIndex = 0;
            lastOutput = 0.0f;
        }
    };

    struct ModulatedAllPassFilter {
        std::vector<float> buffer;
        int baseSize;
        int currentSize;
        int writeIndex = 0;
        float phase = 0.0f;
        float modDepth = 0.0f;
        float modRate = 0.0f;
        float lastOutput = 0.0f;

        ModulatedAllPassFilter(int size = 433) {
            baseSize = size;
            currentSize = size;
            // Extra buffer space for modulation
            buffer.resize(size + 100, 0.0f);
        }

        float process(float input, float coeff, float sampleRate) {
            coeff = juce::jlimit(-0.9f, 0.9f, coeff);

            // Update modulation with smoother transitions
            float prevPhase = phase;
            phase += modRate / sampleRate;
            if (phase >= 1.0f) phase -= 1.0f;

            // Cosine interpolation for smooth transitions
            float t = (phase < prevPhase) ? 0.0f : phase;
            float modFactor = 1.0f + modDepth * std::sin(t * 2.0f * juce::MathConstants<float>::pi);

            // Limit modulation per sample
            int targetSize = static_cast<int>(baseSize * modFactor);
            if (targetSize >= (int)buffer.size()) targetSize = buffer.size() - 1;
            if (targetSize < 1) targetSize = 1;

            // Smooth buffer size transitions
            currentSize = currentSize * 0.99f + targetSize * 0.01f;
            int usedSize = static_cast<int>(currentSize);

            // Processing with modulated delay
            int readIndex = (writeIndex - usedSize + static_cast<int>(buffer.size())) % static_cast<int>(buffer.size());
            float delayedSample = buffer[readIndex];

            float temp = input + (coeff * delayedSample);
            buffer[writeIndex] = temp;
            writeIndex = (writeIndex + 1) % buffer.size();

            float output = delayedSample - (coeff * temp);

            // Smooth transitions
            output = 0.92f * output + 0.08f * lastOutput;
            lastOutput = output;

            return output;
        }

        void setModulation(float depth, float rate) {
			// Ensure parameters are in the range
            modDepth = juce::jlimit(0.0f, 0.3f, depth);
            modRate = juce::jlimit(0.01f, 8.0f, rate);
        }

        void clear() noexcept {
            std::fill(buffer.begin(), buffer.end(), 0.0f);
            writeIndex = 0;
            lastOutput = 0.0f;
            phase = 0.0f;
        }
    };

    const int allPassValues[16] = { 97, 109, 127, 139, 193, 251, 311, 373, 433, 491, 659, 619, 683, 757, 827, 887 };
    std::vector<AllPassFilter> diffusionFilters;
    std::vector<ModulatedAllPassFilter> modulatedDiffusers;
    std::vector<AllPassFilter> postDiffusers;

    // Biquad filter: 2 poles and 2 zeros
    struct BiquadFilter {
        float lastInput = 0.0f;
        float cutoffFreq = 5000.0f;
        // Q factor -> Butterworth
        float q = 0.7071f;

        // First stage
        float b0 = 1.0f, b1 = 0.0f, b2 = 0.0f;
        float a1 = 0.0f, a2 = 0.0f;
        float z1 = 0.0f, z2 = 0.0f;

        // Second stage 
        float b0_2 = 1.0f, b1_2 = 0.0f, b2_2 = 0.0f;
        float a1_2 = 0.0f, a2_2 = 0.0f;
        float z1_2 = 0.0f, z2_2 = 0.0f;

        // Process (24dB/octave)
        float processBiquad(float in) {
            // First stage
            float mid = in * b0 + z1;
            z1 = in * b1 + z2 - a1 * mid;
            z2 = in * b2 - a2 * mid;

            // Second stage
            float out = mid * b0_2 + z1_2;
            z1_2 = mid * b1_2 + z2_2 - a1_2 * out;
            z2_2 = mid * b2_2 - a2_2 * out;

            return out;
        }

        // Coefficients for low-pass filter
        void setLowpass(float frequency, float q, float sampleRate) {
            cutoffFreq = frequency;
            this->q = q;

            float omega = 2.0f * juce::MathConstants<float>::pi * frequency / sampleRate;
            float alpha = std::sin(omega) / (2.0f * q);
            float cosw = std::cos(omega);

            float norm = 1.0f / (1.0f + alpha);

            b0 = ((1.0f - cosw) * 0.5f) * norm;
            b1 = (1.0f - cosw) * norm;
            b2 = ((1.0f - cosw) * 0.5f) * norm;
            a1 = (-2.0f * cosw) * norm;
            a2 = (1.0f - alpha) * norm;
        }

        // Coefficients for high-pass filter
        void setHighpass(float frequency, float q, float sampleRate) {
            cutoffFreq = frequency;
            this->q = q;

            float omega = 2.0f * juce::MathConstants<float>::pi * frequency / sampleRate;
            float alpha = std::sin(omega) / (2.0f * q);
            float cosw = std::cos(omega);

            float norm = 1.0f / (1.0f + alpha);

            b0 = ((1.0f + cosw) * 0.5f) * norm;
            b1 = -(1.0f + cosw) * norm;
            b2 = ((1.0f + cosw) * 0.5f) * norm;
            a1 = (-2.0f * cosw) * norm;
            a2 = (1.0f - alpha) * norm;
        }

        // Reset filter state
        void reset() {
            z1 = z2 = 0.0f;
        }
    };

    std::vector<BiquadFilter> lpfFilters;
    std::vector<BiquadFilter> hpfFilters;

    double sampleRate = 44100.0;

    // Early Reflections
    struct EarlyReflection {
        int delaySamples;
        float gain;
    };

    std::vector<EarlyReflection> earlyReflections;

    std::vector<float> erBuffer;
    int erBufferSize = 0;
    int erWriteIndex = 0;

    AllPassFilter erDiffusion1;
    AllPassFilter erDiffusion2;

    // Soft Limiter
    float softLimit(float input) {
        const float threshold = 0.4f;
        const float limit = 1.0f;   
        const float curve = 1.5f;  

        float abs_x = std::abs(input);
        float sign = input > 0.0f ? 1.0f : -1.0f;

        if (abs_x <= threshold) {
            return input;
        }
        else if (abs_x <= limit) {
            // Cubic soft knee
            float t = (abs_x - threshold) / (limit - threshold);
            float eased = threshold + (limit - threshold) * (t - t * t * t / 3.0f);
            return sign * eased;
        }
        else {
            float overshoot = abs_x - limit;
            return sign * (limit - (1.0f / (overshoot + 1.0f)));
        }
    }

    // Avoid DC Bias
    struct DCBlocker {
        float x1 = 0.0f, x2 = 0.0f;
        float y1 = 0.0f, y2 = 0.0f;

        void reset() {
            x1 = x2 = 0.0f;
            y1 = y2 = 0.0f;
        }

        float process(float input) {
            // Second-order DC blocking filter (pole at 0.995, zero at 1.0)
            const float R = 0.995f;
            float x0 = input;

            // Calculate output: y[n] = x[n] - x[n-2] + R^2 * y[n-2]
            float output = x0 - x2 + R * R * y2;

            // Update variables
            x2 = x1;
            x1 = x0;
            y2 = y1;
            y1 = output;

            return output;
        }
    };

    std::vector<DCBlocker> dcBlockers;

    // Denormal Prevention
    inline float denormalPrevention(float sample) {
        static const float minLevel = 1.0e-8f;
        static const float antiDenormal = 1.0e-8f;

        if (std::abs(sample) < minLevel)
            return antiDenormal * (2.0f * static_cast<float>(rand()) / RAND_MAX - 1.0f);
        return sample;
    }

};
