/*
  ==============================================================================

	This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#pragma once
#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
NewProjectAudioProcessor::NewProjectAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
	: AudioProcessor(BusesProperties()
#if ! JucePlugin_IsMidiEffect
#if ! JucePlugin_IsSynth
		.withInput("Input", juce::AudioChannelSet::stereo(), true)
#endif
		.withOutput("Output", juce::AudioChannelSet::stereo(), true)
#endif
	),
	apvts(*this, nullptr, "Parameters", createParameters())
#endif
{
	// Add voices to the synthesiser
	for (int i = 0; i < 8; ++i)
	{
		synth.addVoice(new juce::SamplerVoice());
	}

	// Load samples
	synth.loadSamples();

	fifoBuffer.setSize(1, fifoSize);

	// Make sure they're the same size
	midiBuffer.resize(midiFifo.getTotalSize());
}

NewProjectAudioProcessor::~NewProjectAudioProcessor()
{
}

//==============================================================================
const juce::String NewProjectAudioProcessor::getName() const
{
	return JucePlugin_Name;
}

bool NewProjectAudioProcessor::acceptsMidi() const
{
#if JucePlugin_WantsMidiInput
	return true;
#else
	return false;
#endif
}

bool NewProjectAudioProcessor::producesMidi() const
{
#if JucePlugin_ProducesMidiOutput
	return true;
#else
	return false;
#endif
}

bool NewProjectAudioProcessor::isMidiEffect() const
{
#if JucePlugin_IsMidiEffect
	return true;
#else
	return false;
#endif
}

double NewProjectAudioProcessor::getTailLengthSeconds() const
{
	return 0.0;
}

int NewProjectAudioProcessor::getNumPrograms()
{
	return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
	// so this should be at least 1, even if you're not really implementing programs.
}

int NewProjectAudioProcessor::getCurrentProgram()
{
	return 0;
}

void NewProjectAudioProcessor::setCurrentProgram(int index)
{
}

const juce::String NewProjectAudioProcessor::getProgramName(int index)
{
	return {};
}

void NewProjectAudioProcessor::changeProgramName(int index, const juce::String& newName)
{
}

//==============================================================================
void NewProjectAudioProcessor::prepareToPlay(double sampleRate, int samplesPerBlock)
{
	synth.setCurrentPlaybackSampleRate(sampleRate);
}

void NewProjectAudioProcessor::releaseResources()
{
	// When playback stops, you can use this as an opportunity to free up any
	// spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool NewProjectAudioProcessor::isBusesLayoutSupported(const BusesLayout& layouts) const
{
#if JucePlugin_IsMidiEffect
	juce::ignoreUnused(layouts);
	return true;
#else
	// This is the place where you check if the layout is supported.
	// In this template code we only support mono or stereo.
	// Some plugin hosts, such as certain GarageBand versions, will only
	// load plugins that support stereo bus layouts.
	if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
		&& layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
		return false;

	// This checks if the input layout matches the output layout
#if ! JucePlugin_IsSynth
	if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
		return false;
#endif

	return true;
#endif
}
#endif

void NewProjectAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
	// NO CONSOLE OUT IN PROCESS BLOCK
	auto totalNumInputChannels = getTotalNumInputChannels();
	auto totalNumOutputChannels = getTotalNumOutputChannels();

	// Clear output channels with no input data
	for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
	{
		if (i < buffer.getNumChannels() && buffer.getNumSamples() > 0)
		{
			buffer.clear(i, 0, buffer.getNumSamples());
		}
	}

	// Merge custom MIDI messages into the incoming MIDI buffer
	int numToRead = midiFifo.getNumReady();
	int start1, size1, start2, size2;
	midiFifo.prepareToRead(numToRead, start1, size1, start2, size2);

	// First Block
	if (size1 > 0)
	{
		for (int i = 0; i < size1; ++i)
			midiMessages.addEvent(midiBuffer[start1 + i], midiBuffer[start1 + i].getTimeStamp());

		midiFifo.finishedRead(size1);
	}

	// Second Block
	if (size2 > 0)
	{
		for (int i = 0; i < size2; ++i)
			midiMessages.addEvent(midiBuffer[start2 + i], midiBuffer[start2 + i].getTimeStamp());

		midiFifo.finishedRead(size2);
	}

	// Process audio
	synth.renderNextBlock(buffer, midiMessages, 0, buffer.getNumSamples());

	// Write to FIFO buffer
	const int numSamples = buffer.getNumSamples();
	const float* writePointer = buffer.getReadPointer(0);

	for (int i = 0; i < numSamples; ++i)
	{
		fifoBuffer.setSample(0, fifoIndex % fifoSize, writePointer[i]);
		fifoIndex++;
	}

	// No MIDI output
	midiMessages.clear();

	// ================================================================

	// Gain Control
	auto localGain = apvts.getRawParameterValue("GAIN")->load();

	for (int channel = 0; channel < buffer.getNumChannels(); ++channel)
	{
		auto* channelData = buffer.getWritePointer(channel);
		for (int sample = 0; sample < buffer.getNumSamples(); ++sample)
			channelData[sample] *= localGain;
	}

	if (auto* reverbEnabled = apvts.getRawParameterValue("REVERB_ENABLED"))
	{
		if (reverbEnabled->load())
		{
			// Get all reverb parameters
			auto predelay = apvts.getRawParameterValue("PREDELAY")->load();
			auto decay = apvts.getRawParameterValue("DECAY")->load();
			auto dryWet = apvts.getRawParameterValue("DRYWET")->load();
			auto diffusion = apvts.getRawParameterValue("DIFFUSION")->load();

			// Save original dry signal if you need to mix it later
			juce::AudioBuffer<float> dryBuffer;
			if (dryWet < 1.0f) { // Only copy if we need to mix dry signal
				dryBuffer.makeCopyOf(buffer);
			}

			// Process audio with reverb
			auto outputs = fdnReverb.process(buffer, predelay, decay, diffusion);

			// Clear the buffer as we'll fill it with the processed signal
			buffer.clear();

			// Mix the output of all delay lines back into the buffer
			for (int channel = 0; channel < buffer.getNumChannels(); ++channel)
			{
				for (int sample = 0; sample < buffer.getNumSamples(); ++sample)
				{
					float mixedSample = 0.0f;

					for (int i = 0; i < outputs.size(); ++i)
						mixedSample += outputs[i][sample];

					mixedSample /= outputs.size();

					if (dryWet < 1.0f)
					{
						float drySample = dryBuffer.getSample(channel, sample);
						mixedSample = drySample * (1.0f - dryWet) + mixedSample * dryWet;
					}

					buffer.setSample(channel, sample, mixedSample);
				}
			}
		}
	}
}

void NewProjectAudioProcessor::addMidiMessage(const juce::MidiMessage& message)
{
	int start1, size1, start2, size2;
	midiFifo.prepareToWrite(1, start1, size1, start2, size2);

	bool written = false;

	if (size1 > 0)
	{
		midiBuffer[start1] = message;
		midiFifo.finishedWrite(1);
		written = true;
	}
	else if (size2 > 0)
	{
		midiBuffer[start2] = message;
		midiFifo.finishedWrite(1);
		written = true;
	}

	// Optionally handle the case where the FIFO is full
	if (!written)
	{
		// Handle buffer overflow if necessary
		// For example, you might log a warning or discard the message
	}
}

juce::AudioProcessorValueTreeState::ParameterLayout NewProjectAudioProcessor::createParameters()
{
	juce::AudioProcessorValueTreeState::ParameterLayout layout;
	std::vector<std::unique_ptr<juce::RangedAudioParameter>> params;

	// Left Controls Parameters
	params.push_back(std::make_unique<juce::AudioParameterFloat>(
		"GAIN", "Gain", 0.0f, 1.0f, 0.5f));
	params.push_back(std::make_unique<juce::AudioParameterFloat>(
		"PITCH_BEND", "Pitch Bend", -2.0f, 2.0f, 0.0f));
	params.push_back(std::make_unique<juce::AudioParameterBool>(
		"ARPEGGIATOR", "Arpeggiator", false));

	// Reverb Parameters
	params.push_back(std::make_unique<juce::AudioParameterFloat>(
		"PREDELAY", "Predelay", 0.0f, 0.1f, 0.0f));
	params.push_back(std::make_unique<juce::AudioParameterFloat>(
		"DECAY", "Decay", 0.5f, 10.0f, 0.5f));
	params.push_back(std::make_unique<juce::AudioParameterFloat>(
		"DRYWET", "Dry/Wet", 0.0f, 1.0f, 0.0f));
	params.push_back(std::make_unique<juce::AudioParameterFloat>(
		"DIFFUSION", "Diffusion", 0.0f, 1.0f, 0.0f));
	params.push_back(std::make_unique<juce::AudioParameterBool>(
		"REVERB_ENABLED", "Reverb Enabled", false));

	// Oscillator Parameters
	params.push_back(std::make_unique<juce::AudioParameterFloat>(
		"OSC1_RANGE", "Osc 1 Range", 0.0f, 1.0f, 0.5f));
	params.push_back(std::make_unique<juce::AudioParameterFloat>(
		"OSC1_SHAPE", "Osc 1 Shape", 0.0f, 2.0f, 0.0f));
	params.push_back(std::make_unique<juce::AudioParameterFloat>(
		"OSC2_RANGE", "Osc 2 Range", 0.0f, 1.0f, 0.5f));
	params.push_back(std::make_unique<juce::AudioParameterFloat>(
		"OSC2_SHAPE", "Osc 2 Shape", 0.0f, 2.0f, 0.0f));
	params.push_back(std::make_unique<juce::AudioParameterBool>(
		"OSC1_ENABLED", "Osc 1 Enabled", false));
	params.push_back(std::make_unique<juce::AudioParameterBool>(
		"OSC2_ENABLED", "Osc 2 Enabled", false));

	// Filter Parameters
	params.push_back(std::make_unique<juce::AudioParameterFloat>(
		"HIGH_CUTOFF", "High Cutoff", 20.0f, 20000.0f, 20000.0f));
	params.push_back(std::make_unique<juce::AudioParameterFloat>(
		"HIGH_SLOPE", "High Slope", 0.0f, 1.0f, 0.5f));
	params.push_back(std::make_unique<juce::AudioParameterFloat>(
		"HIGH_EMPHASIS", "High Emphasis", 0.0f, 1.0f, 0.0f));
	params.push_back(std::make_unique<juce::AudioParameterFloat>(
		"LOW_CUTOFF", "Low Cutoff", 20.0f, 20000.0f, 20.0f));
	params.push_back(std::make_unique<juce::AudioParameterFloat>(
		"LOW_SLOPE", "Low Slope", 0.0f, 1.0f, 0.5f));
	params.push_back(std::make_unique<juce::AudioParameterFloat>(
		"LOW_EMPHASIS", "Low Emphasis", 0.0f, 1.0f, 0.0f));
	params.push_back(std::make_unique<juce::AudioParameterBool>(
		"HIGH_ENABLED", "High Pass Enabled", false));
	params.push_back(std::make_unique<juce::AudioParameterBool>(
		"LOW_ENABLED", "Low Pass Enabled", false));

	return { params.begin(), params.end() };
}



//==============================================================================
bool NewProjectAudioProcessor::hasEditor() const
{
	return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* NewProjectAudioProcessor::createEditor()
{
	return new NewProjectAudioProcessorEditor(*this);
}

//==============================================================================
void NewProjectAudioProcessor::getStateInformation(juce::MemoryBlock& destData)
{
	// You should use this method to store your parameters in the memory block.
	// You could do that either as raw data, or use the XML or ValueTree classes
	// as intermediaries to make it easy to save and load complex data.
}

void NewProjectAudioProcessor::setStateInformation(const void* data, int sizeInBytes)
{
	// You should use this method to restore your parameters from this memory block,
	// whose contents will have been created by the getStateInformation() call.
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
	return new NewProjectAudioProcessor();
}
