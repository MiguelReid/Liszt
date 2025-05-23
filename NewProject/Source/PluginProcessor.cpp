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
	lfo1.setSampleRate(sampleRate);
	lfo2.setSampleRate(sampleRate);
	fdnReverb.prepare(sampleRate);
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

	// Reverb parameters to modulate
	double baseDecay = apvts.getRawParameterValue("DECAY")->load();
	double baseDiffusion = apvts.getRawParameterValue("DIFFUSION")->load();
	double basePredelay = apvts.getRawParameterValue("PREDELAY")->load();

	// Process LFO modulation if enabled
	double osc1Depth = apvts.getRawParameterValue("OSC1_DEPTH")->load();
	int osc1Shape = static_cast<int>(apvts.getRawParameterValue("OSC1_SHAPE")->load());
	int osc1Target = static_cast<int>(apvts.getRawParameterValue("OSC1_TARGET")->load());
	bool osc1Enabled = apvts.getRawParameterValue("OSC1_ENABLED")->load() > 0.5f;

	double osc2Depth = apvts.getRawParameterValue("OSC2_DEPTH")->load();
	int osc2Shape = static_cast<int>(apvts.getRawParameterValue("OSC2_SHAPE")->load());
	int osc2Target = static_cast<int>(apvts.getRawParameterValue("OSC2_TARGET")->load());
	bool osc2Enabled = apvts.getRawParameterValue("OSC2_ENABLED")->load() > 0.5f;

	// Modulate parameters with LFOs if enabled
	double modulatedDiffusion = baseDiffusion;
	double modulatedDecay = baseDecay;
	double modulatedPredelay = basePredelay;

	// In PluginProcessor.cpp, replace the switch statements with these fixed versions:

	if (osc1Enabled) {
		float modValue = lfo1.processLFO(osc1Depth, osc1Shape, osc1Target);

		switch (osc1Target) {
		case 0: // Diffusion (0.0 - 1.0 range)
			modulatedDiffusion = juce::jlimit(0.0, 1.0,
				baseDiffusion + (modValue * 0.5)); // 50% of diffusion range
			break;
		case 1: // Decay (0.8 - 5.0 range)
			modulatedDecay = juce::jlimit(0.8, 5.0,
				baseDecay + (modValue * 2.1)); // 50% of decay range
			break;
		case 2: // Predelay (0.0 - 100.0 range)
			modulatedPredelay = juce::jlimit(0.0, 100.0,
				basePredelay + (modValue * 50.0)); // 50% of predelay range
			break;
		}
	}

	if (osc2Enabled) {
		float modValue = lfo2.processLFO(osc2Depth, osc2Shape, osc2Target);

		switch (osc2Target) {
		case 0: // Diffusion (0.0 - 1.0 range)
			modulatedDiffusion = juce::jlimit(0.0, 1.0,
				baseDiffusion + (modValue * 0.5)); // 50% of diffusion range
			break;
		case 1: // Decay (0.8 - 5.0 range)
			modulatedDecay = juce::jlimit(0.8, 5.0,
				baseDecay + (modValue * 2.1)); // 50% of decay range
			break;
		case 2: // Predelay (0.0 - 100.0 range)
			modulatedPredelay = juce::jlimit(0.0, 100.0,
				basePredelay + (modValue * 50.0)); // 50% of predelay range
			break;
		}
	}


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
			auto dryWet = apvts.getRawParameterValue("DRYWET")->load();

			// HPF and LPF
			auto hpCutoff = apvts.getRawParameterValue("HIGH_CUTOFF")->load();
			auto lpCutoff = apvts.getRawParameterValue("LOW_CUTOFF")->load();

			// Save original dry signal if you need to mix it later
			juce::AudioBuffer<float> dryBuffer;
			if (dryWet < 1.0f) { // Only copy if we need to mix dry signal
				dryBuffer.makeCopyOf(buffer);
			}

			// Process audio with reverb
			auto outputs = fdnReverb.process(buffer, modulatedPredelay, modulatedDecay, modulatedDiffusion, hpCutoff, lpCutoff);

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

	if (buffer.getNumChannels() > 0)
	{
		float visualizationGain = 2.0f;
		const float* readPtr = buffer.getReadPointer(0);

		for (int i = 0; i < numSamples; ++i)
		{
			// Scale the sample just for visualization purposes
			float scaledSample = readPtr[i] * visualizationGain;

			fifoBuffer.setSample(0, fifoIndex % fifoSize, scaledSample);
			fifoIndex++;
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
}

juce::AudioProcessorValueTreeState::ParameterLayout NewProjectAudioProcessor::createParameters()
{
	juce::AudioProcessorValueTreeState::ParameterLayout layout;
	std::vector<std::unique_ptr<juce::RangedAudioParameter>> params;

	// Left Controls Parameters (Gain with SkewFactor)
	params.push_back(std::make_unique<juce::AudioParameterFloat>(
		"GAIN", "Gain", juce::NormalisableRange<float>(0.0f, 3.0f, 0.01f, 0.5f), 1.5f));
	params.push_back(std::make_unique<juce::AudioParameterFloat>(
		"PITCH_BEND", "Pitch Bend", juce::NormalisableRange<float>(-2.0f, 2.0f), 0.0f));
	params.push_back(std::make_unique<juce::AudioParameterBool>(
		"ARPEGGIATOR", "Arpeggiator", false));

	// Reverb Parameters
	params.push_back(std::make_unique<juce::AudioParameterFloat>(
		"PREDELAY", "Predelay", juce::NormalisableRange<float>(0.0f, 100.0f, 1.0f), 50.0f));
	params.push_back(std::make_unique<juce::AudioParameterFloat>(
		"DECAY", "Decay", juce::NormalisableRange<float>(0.8f, 5.0f, 0.1f), 2.5f));
	params.push_back(std::make_unique<juce::AudioParameterFloat>(
		"DRYWET", "Dry/Wet", juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f), 0.5f));
	params.push_back(std::make_unique<juce::AudioParameterFloat>(
		"DIFFUSION", "Diffusion", juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f), 0.5f));
	params.push_back(std::make_unique<juce::AudioParameterBool>(
		"REVERB_ENABLED", "Reverb Enabled", false));


	// Oscillator 1 Parameters
	params.push_back(std::make_unique<juce::AudioParameterFloat>(
		"OSC1_DEPTH", "Osc 1 DEPTH", juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f), 0.5f));
	params.push_back(std::make_unique<juce::AudioParameterFloat>(
		"OSC1_SHAPE", "Osc 1 Shape", juce::NormalisableRange<float>(0.0f, 2.0f, 0.01f), 0.0f));
	params.push_back(std::make_unique<juce::AudioParameterChoice>("OSC1_TARGET", "Osc 1 Target",
		juce::StringArray("Diffusion", "Decay", "Predelay"), 0));

	// Oscillator 2 Parameters
	params.push_back(std::make_unique<juce::AudioParameterFloat>(
		"OSC2_DEPTH", "Osc 2 DEPTH", juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f), 0.5f));
	params.push_back(std::make_unique<juce::AudioParameterFloat>(
		"OSC2_SHAPE", "Osc 2 Shape", juce::NormalisableRange<float>(0.0f, 2.0f, 0.01f), 0.0f));
	params.push_back(std::make_unique<juce::AudioParameterChoice>("OSC2_TARGET", "Osc 2 Target",
		juce::StringArray("Diffusion", "Decay", "Predelay"), 1));

	// Oscillator Enable Parameters
	params.push_back(std::make_unique<juce::AudioParameterBool>(
		"OSC1_ENABLED", "Osc 1 Enabled", false));
	params.push_back(std::make_unique<juce::AudioParameterBool>(
		"OSC2_ENABLED", "Osc 2 Enabled", false));


	// High-Pass Filter Parameters
	params.push_back(std::make_unique<juce::AudioParameterFloat>(
		"HIGH_CUTOFF", "High Cutoff", juce::NormalisableRange<float>(20.0f, 150.0f, 2.0f), 120.0f));

	// Low-Pass Filter Parameters
	params.push_back(std::make_unique<juce::AudioParameterFloat>(
		"LOW_CUTOFF", "Low Cutoff", juce::NormalisableRange<float>(5000.0f, 16000.0f, 160.0f), 5000.0f));

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
