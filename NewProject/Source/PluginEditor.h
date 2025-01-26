/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"
#include "WaveScreen.h"
#include "LeftControls.h"
#include "ReverbControls.h"
#include "OscillatorControls.h"
#include "FilterControls.h"

//==============================================================================
/**
*/
class NewProjectAudioProcessorEditor  : public juce::AudioProcessorEditor, private juce::MidiKeyboardStateListener, private juce::Timer
{
public:
    NewProjectAudioProcessorEditor (NewProjectAudioProcessor&);
    ~NewProjectAudioProcessorEditor() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;

    void timerCallback() override;

private:
    NewProjectAudioProcessor& audioProcessor;

    // Keyboard
    juce::MidiKeyboardState keyboardState;
    juce::MidiKeyboardComponent keyboardComponent;
    void handleNoteOn(juce::MidiKeyboardState*, int midiChannel, int midiNoteNumber, float velocity) override;
    void handleNoteOff(juce::MidiKeyboardState*, int midiChannel, int midiNoteNumber, float velocity) override;

	WaveScreen waveScreen;
    juce::ToggleButton arpegiattorButton;
    juce::Slider pitchBendSlider;
    LeftControls leftControls;
    ReverbControls reverbControls;
    OscillatorControls oscillatorControls;
	FilterControls filterControls;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (NewProjectAudioProcessorEditor)
};
