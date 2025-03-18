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

    std::vector<std::vector<float>> FDNReverb::process(juce::AudioBuffer<float>& buffer, double predelay, double decay, double diffusion);
    void prepare(double newSampleRate);

private:
    // DelayLines
    std::vector<std::unique_ptr<CustomDelayLine>> delayLines;
    static constexpr int numDelayLines = 8;
    const int primeDelays[numDelayLines] = { 1087, 1283, 1511, 1747, 1997, 2269, 2539, 2797 };
    PredelayLine predelayBuffer{ 96000 }; // Maximum 2 seconds at 48kHz

    // Normalised hadamard matrix
    std::vector<std::vector<float>> FDNReverb::hadamard(const std::vector<std::vector<float>>& delayOutputs);
    const std::array<std::array<float, numDelayLines>, numDelayLines> hadamardMatrix = { {
        {  0.354f,  0.354f,  0.354f,  0.354f,  0.354f,  0.354f,  0.354f,  0.354f },
        {  0.354f, -0.354f,  0.354f, -0.354f,  0.354f, -0.354f,  0.354f, -0.354f },
        {  0.354f,  0.354f, -0.354f, -0.354f,  0.354f,  0.354f, -0.354f, -0.354f },
        {  0.354f, -0.354f, -0.354f,  0.354f,  0.354f, -0.354f, -0.354f,  0.354f },
        {  0.354f,  0.354f,  0.354f,  0.354f, -0.354f, -0.354f, -0.354f, -0.354f },
        {  0.354f, -0.354f,  0.354f, -0.354f, -0.354f,  0.354f, -0.354f,  0.354f },
        {  0.354f,  0.354f, -0.354f, -0.354f, -0.354f, -0.354f,  0.354f,  0.354f },
        {  0.354f, -0.354f, -0.354f,  0.354f, -0.354f,  0.354f,  0.354f, -0.354f }
    } };

    // 8x8 Householder matrix (normalized)
    const std::array<std::array<float, numDelayLines>, numDelayLines> householderMatrix = { {
        {  0.75f, -0.25f, -0.25f, -0.25f, -0.25f, -0.25f, -0.25f, -0.25f },
        { -0.25f,  0.75f, -0.25f, -0.25f, -0.25f, -0.25f, -0.25f, -0.25f },
        { -0.25f, -0.25f,  0.75f, -0.25f, -0.25f, -0.25f, -0.25f, -0.25f },
        { -0.25f, -0.25f, -0.25f,  0.75f, -0.25f, -0.25f, -0.25f, -0.25f },
        { -0.25f, -0.25f, -0.25f, -0.25f,  0.75f, -0.25f, -0.25f, -0.25f },
        { -0.25f, -0.25f, -0.25f, -0.25f, -0.25f,  0.75f, -0.25f, -0.25f },
        { -0.25f, -0.25f, -0.25f, -0.25f, -0.25f, -0.25f,  0.75f, -0.25f },
        { -0.25f, -0.25f, -0.25f, -0.25f, -0.25f, -0.25f, -0.25f,  0.75f }
    } };

    struct AllPassFilter {
        std::vector<float> buffer;
        int bufferSize = 0;
        int writeIndex = 0;

        AllPassFilter(int size = 277) { // Use a prime number for buffer size
            buffer.resize(size, 0.0f);
            bufferSize = size;
        }

        float process(float input, float coeff) {
            coeff = juce::jlimit(-0.9f, 0.9f, coeff);

            int readIndex = (writeIndex - bufferSize + static_cast<int>(buffer.size())) % static_cast<int>(buffer.size());
            float delayedSample = buffer[readIndex];

            float temp = input + (coeff * delayedSample);
            buffer[writeIndex] = temp;
            writeIndex = (writeIndex + 1) % buffer.size();

            return delayedSample - (coeff * temp);
        }

        // New clear method to reset the filter state
        void clear() noexcept {
            std::fill(buffer.begin(), buffer.end(), 0.0f);
            writeIndex = 0;
        }
    };


    const int allPassValues[8] = { 277, 379, 419, 479, 547, 607, 661, 739 };

    std::vector<AllPassFilter> diffusionFilters;

	// Biquad filter: 2 poles and 2 zeros
    struct BiquadFilter {
        float lastInput = 0.0f;
        float cutoffFreq = 5000.0f;  // Store current cutoff frequency
        float q = 0.7071f;          // Default Q factor (Butterworth)

        // First stage
        float b0 = 1.0f, b1 = 0.0f, b2 = 0.0f;
        float a1 = 0.0f, a2 = 0.0f;
        float z1 = 0.0f, z2 = 0.0f;

        // Second stage 
        float b0_2 = 1.0f, b1_2 = 0.0f, b2_2 = 0.0f;
        float a1_2 = 0.0f, a2_2 = 0.0f;
        float z1_2 = 0.0f, z2_2 = 0.0f;

        // Process using cascaded biquads (24dB/octave)
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

        // Set coefficients for low-pass filter
        void setLowpass(float frequency, float q, float sampleRate) {
            // Store current settings
            cutoffFreq = frequency;
            this->q = q;

            // Avoid recalculating if the parameters haven't changed significantly
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

        // Set coefficients for high-pass filter
        void setHighpass(float frequency, float q, float sampleRate) {
            // Store current settings
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

    // Early reflections implementation
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
        // Cubic soft clipper for smooth limiting
        if (input > 1.0f)
            return 1.0f - (1.0f / (input + 1.0f)); // Asymptotic approach to 1.0
        else if (input < -1.0f)
            return -1.0f + (1.0f / (-input + 1.0f)); // Asymptotic approach to -1.0
        else if (input > 0.4f)
            return 0.4f + (0.6f * (input - 0.4f) / (0.6f + (input - 0.4f)));
        else if (input < -0.4f)
            return -0.4f + (0.6f * (input + 0.4f) / (0.6f - (input + 0.4f)));
        else
            return input; // Pass through unaffected
    }

	// Avoid static noise buildup with DC blocking
    struct DCBlocker {
        float x1 = 0.0f, x2 = 0.0f;
        float y1 = 0.0f, y2 = 0.0f;

        void reset() {
            x1 = x2 = 0.0f;
            y1 = y2 = 0.0f;
        }

        float process(float input) {
            // Second-order DC blocking filter (pole at 0.995, zero at 1.0)
            const float R = 0.995f; // Pole radius - closer to 1 gives narrower notch

            // Store current input
            float x0 = input;

            // Calculate output: y[n] = x[n] - x[n-2] + R^2 * y[n-2]
            float output = x0 - x2 + R * R * y2;

            // Update state variables
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