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
    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.
    setSize (677, 550);
    addAndMakeVisible(keyboardComponent);
	addAndMakeVisible(waveScreen);
    addAndMakeVisible(leftControls);

    // 2 Octaves
    int startNote = 36; // C4 (Middle C)
    int endNote = startNote + 60;
    keyboardComponent.setAvailableRange(startNote, endNote);
    keyboardState.addListener(this);
}

NewProjectAudioProcessorEditor::~NewProjectAudioProcessorEditor()
{
    keyboardState.removeListener(this);
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

}

// Positions of any subcomponents
void NewProjectAudioProcessorEditor::resized()
{
    auto bounds = getLocalBounds();

    // Space on the left for extra controls
    auto controlAreaWidth = 100;
    auto keyboardHeight = 130; // Altura fija para el teclado

    leftControls.setBounds(bounds.removeFromLeft(controlAreaWidth).withHeight(keyboardHeight).withY(bounds.getBottom() - keyboardHeight));

    // Keyboard Position
    keyboardComponent.setBounds(bounds.removeFromBottom(keyboardHeight));

    // WaveScreen Position
    auto screenHeight = 80;
    auto screenWidth = 130;
    auto screenX = (getWidth() - screenWidth) / 2;
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