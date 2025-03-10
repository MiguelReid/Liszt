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
    const int primeDelays[numDelayLines] = { 1031, 1327, 1523, 1871, 2053, 2311, 2539, 2803 };

    // Predelay Lines
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

    // First, define the AllPassFilter properly in your header file
    struct AllPassFilter {
        float buffer1[2] = { 0.0f, 0.0f };
        float buffer2[2] = { 0.0f, 0.0f };

        // Process with stability safeguards
        float process(float input, float coeff) {
            // Ensure coefficient is in stable range
            coeff = juce::jlimit(-0.9f, 0.9f, coeff);

            // First all-pass stage with DC blocking
            float temp = input + (-coeff * buffer1[0]);
            float output1 = buffer1[0] + (coeff * temp);
            buffer1[0] = buffer1[1];
            buffer1[1] = temp;

            // Second all-pass stage 
            temp = output1 + (-coeff * buffer2[0]);
            float output2 = buffer2[0] + (coeff * temp);
            buffer2[0] = buffer2[1];
            buffer2[1] = temp;

            // Apply safety limiter to prevent instability
            return juce::jlimit(-1.0f, 1.0f, output2);
        }
    };

    std::vector<AllPassFilter> diffusionFilters;

	// Biquad filter
    struct BiquadFilter {
        float b0 = 1.0f, b1 = 0.0f, b2 = 0.0f;
        float a1 = 0.0f, a2 = 0.0f;
        float z1 = 0.0f, z2 = 0.0f;
        float lastInput = 0.0f;
        float cutoffFreq = 5000.0f;  // Store current cutoff frequency
        float q = 0.7071f;          // Default Q factor (Butterworth)

        // Process using proper biquad implementation
        float processBiquad(float in) {
            float out = in * b0 + z1;
            z1 = in * b1 + z2 - a1 * out;
            z2 = in * b2 - a2 * out;
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

        // Legacy one-pole filter with slew limiting
        float process(float in, float cutoff) {
            const float maxChange = 0.01f;
            float change = in - lastInput;
            if (std::abs(change) > maxChange) {
                in = lastInput + maxChange * (change > 0 ? 1.0f : -1.0f);
            }
            lastInput = in;

            // Apply simple low-pass
            z1 = z1 + cutoff * (in - z1);
            return z1;
        }
    };


    std::vector<BiquadFilter> lpfFilters;

    struct LFO {
        float phase = 0.0f;

        float process(float frequency, float sampleRate) {
            phase += frequency / sampleRate;
            if (phase > 1.0f) phase -= 1.0f;
            return 0.5f * (1.0f + std::sin(2.0f * juce::MathConstants<float>::pi * phase));
        }
    };

    std::vector<LFO> lfos;
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
        float x1 = 0.0f, y1 = 0.0f;

        float process(float input) {
            float output = input - x1 + 0.995f * y1;
            x1 = input;
            y1 = output;
            return output;
        }
    };

    std::vector<DCBlocker> dcBlockers;

    // Denormal Prevention
    inline float denormalPrevention(float sample) const {
        static constexpr float antiDenormal = 1.0e-9f;
        return sample + antiDenormal;
    }
};