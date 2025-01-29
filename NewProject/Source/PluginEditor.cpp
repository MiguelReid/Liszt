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
    setSize (880, 380);
    startTimerHz(30); // Adjust refresh rate as needed
    addAndMakeVisible(keyboardComponent);
	addAndMakeVisible(waveScreen);
    addAndMakeVisible(leftControls);
    addAndMakeVisible(reverbControls);
    addAndMakeVisible(oscillatorControls);
	addAndMakeVisible(filterControls);

    // C0 to F6
    keyboardComponent.setAvailableRange(24, 101);
    keyboardState.addListener(this);
}

NewProjectAudioProcessorEditor::~NewProjectAudioProcessorEditor()
{
    keyboardState.removeListener(this);
}

//==============================================================================
void NewProjectAudioProcessorEditor::paint (juce::Graphics& g)
{
    g.fillAll(juce::Colour::fromRGB(109, 70, 44));
    juce::ColourGradient gradient(juce::Colour::fromRGB(60, 35, 20), 0, 0,
        juce::Colour::fromRGB(150, 90, 55), getWidth(), getHeight(),false);
    g.setGradientFill(gradient);
    g.fillRect(getLocalBounds());

    g.setColour(juce::Colours::black);
    g.drawLine(reverbControls.getRight() + 25, 0, reverbControls.getRight() + 25, getHeight(), 2.0f);
    g.drawLine(oscillatorControls.getRight() + 25, 0, oscillatorControls.getRight() + 25, getHeight(), 2.0f);
    g.drawLine(filterControls.getRight() + 25, 0, filterControls.getRight() + 25, getHeight(), 2.0f);

    int lineY = waveScreen.getY() - 10;
    g.drawLine(filterControls.getRight() + 25, lineY, getWidth(), lineY, 2.0f);
}

void NewProjectAudioProcessorEditor::resized()
{
    auto bounds = getLocalBounds();

    // Space on the left for extra controls
    auto controlAreaWidth = 135;
    auto keyboardHeight = 130;
    auto reverbControlsHeight = 200;
    int oscillatorControlsHeight = 200;

    // Position ReverbControls lower down and to the right
    auto leftControlArea = bounds.removeFromLeft(controlAreaWidth);
    auto reverbOffsetX = 20; // Horizontal offset from the left edge
    auto reverbOffsetY = 30; // Vertical offset from the top
    reverbControls.setBounds(leftControlArea
        .removeFromTop(reverbControlsHeight)
        .translated(reverbOffsetX, reverbOffsetY));

    // Position OscillatorControls to the right of ReverbControls
    oscillatorControls.setBounds(reverbControls.getRight() + 50, reverbControls.getY(), 200, oscillatorControlsHeight);

	// Position FilterControls to the right of OscillatorControls
	filterControls.setBounds(oscillatorControls.getRight() + 50, oscillatorControls.getY(), 180, oscillatorControlsHeight);

    // Position LeftControls at the bottom left
    leftControls.setBounds(leftControlArea.withY(getHeight() - keyboardHeight).withHeight(keyboardHeight));

    // Position the keyboard at the bottom
    keyboardComponent.setBounds(bounds.removeFromBottom(keyboardHeight));

    // WaveScreen Position
    auto screenHeight = 70;
    auto screenWidth = 170;
    auto screenX = getWidth() - screenWidth - 20;
    auto screenY = keyboardComponent.getY() - screenHeight - 10;
    waveScreen.setBounds(screenX, screenY, screenWidth, screenHeight);
}


void NewProjectAudioProcessorEditor::handleNoteOn(juce::MidiKeyboardState*, int midiChannel, int midiNoteNumber, float velocity)
{
    // Send a note-on message to the processor
    juce::MidiMessage message = juce::MidiMessage::noteOn(midiChannel, midiNoteNumber, velocity);
    audioProcessor.addMidiMessage(message);
	DBG("Note On: " << midiNoteNumber);
}

void NewProjectAudioProcessorEditor::handleNoteOff(juce::MidiKeyboardState*, int midiChannel, int midiNoteNumber, float velocity)
{
    // Send a note-off message to the processor
    juce::MidiMessage message = juce::MidiMessage::noteOff(midiChannel, midiNoteNumber);
    audioProcessor.addMidiMessage(message);
}

void NewProjectAudioProcessorEditor::timerCallback()
{
    if (waveScreen.getVisualiserStatus()) {
        int availableSamples = audioProcessor.fifoIndex.load();
        if (availableSamples >= audioProcessor.fifoSize)
        {
            availableSamples = audioProcessor.fifoSize;
        }

        if (availableSamples > 0)
        {
            juce::AudioBuffer<float> tempBuffer(1, availableSamples);

            for (int i = 0; i < availableSamples; ++i)
            {
                tempBuffer.setSample(0, i, audioProcessor.fifoBuffer.getSample(0, i));
            }

            // Push samples into the visualiser
            waveScreen.audioVisualiser.pushBuffer(tempBuffer);

            // Reset FIFO index
            audioProcessor.fifoIndex.store(0);
        }
    }
}