/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#pragma once
#include <JuceHeader.h>
#include "Synth.h"
#include "WaveScreen.h"
#include <juce_audio_basics/juce_audio_basics.h>
#include "ReverbControls.h"
#include "FDNReverb.h"

//==============================================================================
/**
*/
class NewProjectAudioProcessor  : public juce::AudioProcessor
{
public:

    // FIFO
    juce::AudioSampleBuffer fifoBuffer;
    std::atomic<int> fifoIndex{ 0 };
    static constexpr int fifoSize = 48000; // Adjust size as needed

    //==============================================================================
    NewProjectAudioProcessor();
    ~NewProjectAudioProcessor() override;

    //==============================================================================
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

   #ifndef JucePlugin_PreferredChannelConfigurations
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
   #endif

    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    //==============================================================================
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    //==============================================================================
    const juce::String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    //==============================================================================
    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int index) override;
    const juce::String getProgramName (int index) override;
    void changeProgramName (int index, const juce::String& newName) override;

    //==============================================================================
    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

	//==============================================================================
    void addMidiMessage(const juce::MidiMessage& message);

	// ==============================================================================
    void setGain(float newGain) { gain = newGain; }
    float getGain() const { return gain; }

private:
    //==============================================================================
    Synth synth;
    FDNReverb fdnReverb;
	ReverbControls reverbControls;
    juce::AbstractFifo midiFifo{ 1024 }; // Size the FIFO as needed
    std::vector<juce::MidiMessage> midiBuffer;
    float gain = 1.0f;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (NewProjectAudioProcessor)
};
