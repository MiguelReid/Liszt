/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
NewProjectAudioProcessorEditor::NewProjectAudioProcessorEditor (NewProjectAudioProcessor& p)
	: AudioProcessorEditor(&p), audioProcessor(p), keyboardComponent(keyboardState, juce::MidiKeyboardComponent::horizontalKeyboard)
{
    setSize (677, 350);
    addAndMakeVisible(keyboardComponent);
	addAndMakeVisible(waveScreen);
    addAndMakeVisible(leftControls);
    addAndMakeVisible(reverbControls);

    // Keyboard Settings
    int startNote = 36; // 60 is middle C
    int endNote = startNote + 60;
    keyboardComponent.setAvailableRange(startNote, endNote);
    keyboardState.addListener(this);
}

NewProjectAudioProcessorEditor::~NewProjectAudioProcessorEditor()
{
    keyboardState.removeListener(this);
    /*
    arpeggiatorButton.setLookAndFeel(nullptr);
    */
}

//==============================================================================
void NewProjectAudioProcessorEditor::paint (juce::Graphics& g)
{
    g.fillAll(juce::Colour::fromRGB(109, 70, 44));  // Rich wood brown

    // Add subtle shading for a 3D effect
    juce::ColourGradient gradient(juce::Colour::fromRGB(60, 35, 20), 0, 0,
        juce::Colour::fromRGB(150, 90, 55), getWidth(), getHeight(),false);
    g.setGradientFill(gradient);
    g.fillRect(getLocalBounds());

    g.setColour(juce::Colours::black);
    g.drawLine(reverbControls.getRight() + 40, 0, reverbControls.getRight() + 40, getHeight(), 2.0f);
}

void NewProjectAudioProcessorEditor::resized()
{
    auto bounds = getLocalBounds();

    // Space on the left for extra controls
    auto controlAreaWidth = 120;
    auto keyboardHeight = 130;       // Fixed height for the keyboard
    auto reverbControlsHeight = 200; // Height for ReverbControls

    // Position ReverbControls lower down and to the right
    auto leftControlArea = bounds.removeFromLeft(controlAreaWidth);
    auto reverbOffsetX = 23; // Horizontal offset from the left edge
    auto reverbOffsetY = 30; // Vertical offset from the top
    reverbControls.setBounds(leftControlArea
        .removeFromTop(reverbControlsHeight)
        .translated(reverbOffsetX, reverbOffsetY)); // Apply offset

    // Position LeftControls explicitly at the bottom left
    leftControls.setBounds(leftControlArea.withY(getHeight() - keyboardHeight).withHeight(keyboardHeight));

    // Position the keyboard at the bottom
    keyboardComponent.setBounds(bounds.removeFromBottom(keyboardHeight));

    // WaveScreen Position
    auto screenHeight = 100;
    auto screenWidth = 160;
    auto screenX = (getWidth() - screenWidth) - 20;
    auto screenY = keyboardComponent.getY() - screenHeight - 15;
    waveScreen.setBounds(screenX, screenY, screenWidth, screenHeight);
}


void NewProjectAudioProcessorEditor::handleNoteOn(juce::MidiKeyboardState*, int midiChannel, int midiNoteNumber, float velocity)
{
    // Send a note-on message to the processor
    juce::MidiMessage message = juce::MidiMessage::noteOn(midiChannel, midiNoteNumber, velocity);
    audioProcessor.addMidiMessage(message);
}

void NewProjectAudioProcessorEditor::handleNoteOff(juce::MidiKeyboardState*, int midiChannel, int midiNoteNumber, float velocity)
{
    // Send a note-off message to the processor
    juce::MidiMessage message = juce::MidiMessage::noteOff(midiChannel, midiNoteNumber);
    audioProcessor.addMidiMessage(message);
}