#pragma once
#include <JuceHeader.h>

class CustomSamplerVoice : public juce::SamplerVoice
{
public:
	CustomSamplerVoice() {}

	bool canPlaySound(juce::SynthesiserSound* sound) override
	{
		return dynamic_cast<juce::SamplerSound*> (sound) != nullptr;
	}

	void startNote(int midiNoteNumber, float velocity, juce::SynthesiserSound* sound, int currentPitchWheelPosition) override
	{
		juce::SamplerVoice::startNote(midiNoteNumber, velocity, sound, currentPitchWheelPosition);

		noteOnTime = juce::Time::getMillisecondCounterHiRes();
		sourceSamplePosition = 0.0; // Reset sample position

		float attack = juce::jmap(velocity, 0.0f, 1.0f, 0.1f, 0.01f);

		adsrParams.attack = attack;
		adsrParams.decay = 0.1f;
		adsrParams.sustain = 0.7f;
		adsrParams.release = 0.2f; 

		adsr.setParameters(adsrParams);
		adsr.noteOn();
	}

	void stopNote(float velocity, bool allowTailOff) override
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

	void renderNextBlock(juce::AudioBuffer<float>& outputBuffer, int startSample, int numSamples) override
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

				// Calculate envelope and get next sample
				float envelopeValue = adsr.getNextSample();

				// Handle sample position with interpolation
				int pos = (int)sourceSamplePosition;
				float alpha = (float)(sourceSamplePosition - pos);
				int nextPos = pos + 1;

				if (nextPos >= totalLength)
				{
					stopNote(0.0f, false);
					break;
				}

				// Read sample from input data
				float inputSample = 0.0f;
				for (int channel = 0; channel < numInputChannels; ++channel)
				{
					auto* inData = data.getReadPointer(channel);
					float sample = inData[pos] * (1.0f - alpha) + inData[nextPos] * alpha;
					inputSample += sample;
				}
				inputSample /= numInputChannels; // Average if more than one channel

				// Write to all output channels
				for (int channel = 0; channel < numOutputChannels; ++channel)
				{
					auto* outData = outputBuffer.getWritePointer(channel, startSample);
					outData[i] += inputSample * envelopeValue;
				}

				sourceSamplePosition += 1.0; // Increment position
			}
		}
	}

private:
	juce::ADSR adsr;
	juce::ADSR::Parameters adsrParams;
	double noteOnTime = 0.0;
	double sourceSamplePosition = 0.0; // Track sample position
};