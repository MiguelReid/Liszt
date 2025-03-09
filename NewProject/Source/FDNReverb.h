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

// Add this to your FDNReverb class
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
    std::vector<std::unique_ptr<CustomDelayLine>> delayLines;
    static constexpr int numDelayLines = 4;

    // Normalised hadamard matrix
    std::vector<std::vector<float>> FDNReverb::hadamard(const std::vector<std::vector<float>>& delayOutputs);
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

    struct BiquadFilter {
        float b0 = 1.0f, b1 = 0.0f, b2 = 0.0f;
        float a1 = 0.0f, a2 = 0.0f;
        float z1 = 0.0f, z2 = 0.0f;

        void setLowpass(float frequency, float Q, float sampleRate) {
            float omega = 2.0f * juce::MathConstants<float>::pi * frequency / sampleRate;
            float alpha = std::sin(omega) / (2.0f * Q);

            float cosw = std::cos(omega);
            float scale = 1.0f / (1.0f + alpha);

            b0 = (1.0f - cosw) * 0.5f * scale;
            b1 = (1.0f - cosw) * scale;
            b2 = (1.0f - cosw) * 0.5f * scale;
            a1 = -2.0f * cosw * scale;
            a2 = (1.0f - alpha) * scale;
        }

        float process(float in, float cutoff) {
            // Simple one-pole lowpass for now, using the cutoff parameter
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
};
