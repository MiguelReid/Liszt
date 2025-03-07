/*
  ==============================================================================

    FDNReverb.h
    Created: 18 Feb 2025 10:24:35am
    Author:  mikey

  ==============================================================================
*/

#pragma once
<<<<<<< Updated upstream
=======
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

class FDNReverb
{
public:
    FDNReverb();
    ~FDNReverb();

    std::vector<std::vector<float>> FDNReverb::process(juce::AudioBuffer<float>& buffer, double predelay, double decay, double diffusion, double dryWet);

private:
    std::vector<std::unique_ptr<CustomDelayLine>> delayLines;
	std::vector<std::vector<float>> FDNReverb::hadamard(const std::vector<std::vector<float>>& delayOutputs);
    static constexpr int numDelayLines = 4;

    // Normalised hadamard matrix
    const std::array<std::array<float, numDelayLines>, numDelayLines> hadamardMatrix = { {
        {  0.5f,  0.5f,  0.5f,  0.5f },
        {  0.5f, -0.5f,  0.5f, -0.5f },
        {  0.5f,  0.5f, -0.5f, -0.5f },
        {  0.5f, -0.5f, -0.5f,  0.5f }
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

    struct SimpleLPF
    {
        float z1 = 0.0f; // One-pole memory

        float process(float input, float cutoff)
        {
            // Simple one-pole filter y[n] = (1-a)*x[n] + a*y[n-1]
            // cutoff in range (0.0 .. 1.0)
            float output = z1 + cutoff * (input - z1);
            z1 = output;
            return output;
        }
    };

    std::vector<SimpleLPF> lpfFilters;
};
>>>>>>> Stashed changes
