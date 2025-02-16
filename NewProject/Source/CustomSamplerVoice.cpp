#include "CustomSamplerVoice.h"

CustomSamplerVoice::CustomSamplerVoice()
{
}

bool CustomSamplerVoice::canPlaySound(juce::SynthesiserSound* sound)
{
    return dynamic_cast<juce::SamplerSound*>(sound) != nullptr;
}

void CustomSamplerVoice::startNote(int midiNoteNumber, float velocity,
    juce::SynthesiserSound* sound, int currentPitchWheelPosition)
{
    juce::SamplerVoice::startNote(midiNoteNumber, velocity, sound, currentPitchWheelPosition);

    noteOnTime = juce::Time::getMillisecondCounterHiRes();
    sourceSamplePosition = 0.0;

    float attack = juce::jmap(velocity, 0.0f, 1.0f, 0.1f, 0.01f);

    adsrParams.attack = attack;
    adsrParams.decay = 0.1f;
    adsrParams.sustain = 0.7f;
    adsrParams.release = 0.2f;

    adsr.setParameters(adsrParams);
    adsr.noteOn();
}

void CustomSamplerVoice::stopNote(float velocity, bool allowTailOff)
{
    double noteOffTime = juce::Time::getMillisecondCounterHiRes();
    double noteDuration = (noteOffTime - noteOnTime) / 1000.0;

    float release = juce::jmap(static_cast<float>(noteDuration), 0.0f, 2.0f, 0.1f, 0.3f);

    adsrParams.release = release;
    adsr.setParameters(adsrParams);

    adsr.noteOff();

    if (!allowTailOff || !adsr.isActive())
        clearCurrentNote();
}

void CustomSamplerVoice::renderNextBlock(juce::AudioBuffer<float>& outputBuffer,
    int startSample, int numSamples)
{
    if (auto* playingSound = dynamic_cast<juce::SamplerSound*>(getCurrentlyPlayingSound().get()))
    {
        auto& data = *playingSound->getAudioData();
        const int numInputChannels = data.getNumChannels();
        const int totalLength = data.getNumSamples();
        const int numOutputChannels = outputBuffer.getNumChannels();

        for (int i = 0; i < numSamples; ++i)
        {
            if (!adsr.isActive())
            {
                clearCurrentNote();
                break;
            }

            float envelopeValue = adsr.getNextSample();
            int pos = static_cast<int>(sourceSamplePosition);
            float alpha = static_cast<float>(sourceSamplePosition - pos);
            int nextPos = pos + 1;

            if (nextPos >= totalLength)
            {
                stopNote(0.0f, false);
                break;
            }

            float inputSample = 0.0f;
            for (int channel = 0; channel < numInputChannels; ++channel)
            {
                auto* inData = data.getReadPointer(channel);
                float sample = inData[pos] * (1.0f - alpha) + inData[nextPos] * alpha;
                inputSample += sample;
            }
            inputSample /= numInputChannels;

            for (int channel = 0; channel < numOutputChannels; ++channel)
            {
                auto* outData = outputBuffer.getWritePointer(channel, startSample);
                outData[i] += inputSample * envelopeValue;
            }

            sourceSamplePosition += 1.0;
        }
    }
}
