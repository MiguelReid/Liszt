#pragma once
#include <JuceHeader.h>

class CustomSamplerVoice : public juce::SamplerVoice
{
public:
    CustomSamplerVoice();

    bool canPlaySound(juce::SynthesiserSound* sound) override;
    void startNote(int midiNoteNumber, float velocity,
        juce::SynthesiserSound* sound, int currentPitchWheelPosition) override;
    void stopNote(float velocity, bool allowTailOff) override;
    void renderNextBlock(juce::AudioBuffer<float>& outputBuffer,
        int startSample, int numSamples) override;

private:
    juce::ADSR adsr;
    juce::ADSR::Parameters adsrParams;
    double noteOnTime = 0.0;
    double sourceSamplePosition = 0.0;
};
