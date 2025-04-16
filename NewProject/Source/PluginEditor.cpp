/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
NewProjectAudioProcessorEditor::NewProjectAudioProcessorEditor (NewProjectAudioProcessor& p)
	: AudioProcessorEditor(&p), audioProcessor(p), keyboardComponent(keyboardState, juce::MidiKeyboardComponent::horizontalKeyboard), leftControls(p.apvts), reverbControls(p.apvts), oscillatorControls(p.apvts), waveScreen()
{
    setSize (770, 375);
    startTimerHz(30); // Adjust refresh rate as needed
    addAndMakeVisible(keyboardComponent);
	addAndMakeVisible(waveScreen);
    addAndMakeVisible(leftControls);
    addAndMakeVisible(reverbControls);
    addAndMakeVisible(oscillatorControls);

    // C0 to F6
    keyboardComponent.setAvailableRange(24, 101);
    keyboardState.addListener(this);
}

NewProjectAudioProcessorEditor::~NewProjectAudioProcessorEditor()
{
    keyboardState.removeListener(this);
}

//==============================================================================
void NewProjectAudioProcessorEditor::resized()
{
    auto bounds = getLocalBounds();

    // Space on the left for extra controls - this will be for leftControls only
    auto leftControlsWidth = 135;
    auto reverbControlsWidth = 220;
    auto keyboardHeight = 130;
    auto reverbControlsHeight = 195;
    int oscillatorControlsHeight = 200;

    // First remove the area for leftControls
    auto leftControlArea = bounds.removeFromLeft(leftControlsWidth);

    // Position LeftControls at the bottom left with its own width
    leftControls.setBounds(leftControlArea.withY(getHeight() - keyboardHeight).withHeight(keyboardHeight));

    // Position ReverbControls with its own width, not depending on leftControlArea
    auto reverbOffsetX = 30;
    auto reverbOffsetY = 30;
    reverbControls.setBounds(reverbOffsetX, reverbOffsetY, reverbControlsWidth, reverbControlsHeight);

    // Position OscillatorControls to the right of ReverbControls
    oscillatorControls.setBounds(reverbControls.getRight() + 60,
        reverbControls.getY(), // Align with reverbControls
        200,
        oscillatorControlsHeight);

    // Position the keyboard at the bottom
    keyboardComponent.setBounds(bounds.removeFromBottom(keyboardHeight));

    // WaveScreen Position (adjusted to align with oscillatorControls)
    auto screenHeight = 70;
    auto screenWidth = 170;
    waveScreen.setBounds(oscillatorControls.getRight() + 60,
        oscillatorControls.getY(), // Same Y as oscillatorControls
        screenWidth,
        screenHeight);
}


void NewProjectAudioProcessorEditor::paint(juce::Graphics& g)
{
    // Fill background with gradient
    g.fillAll(juce::Colour::fromRGB(109, 70, 44));
    juce::ColourGradient gradient(juce::Colour::fromRGB(60, 35, 20), 0, 0,
        juce::Colour::fromRGB(150, 90, 55), getWidth(), getHeight(), false);
    g.setGradientFill(gradient);
    g.fillRect(getLocalBounds());

    g.setColour(juce::Colours::black);

    // Section 1: reverbControls and filterControls together
    juce::Rectangle<float> section1;
    section1.setX(reverbControls.getX() - 15);
    section1.setTop(reverbControls.getY() - 15);
    section1.setRight(reverbControls.getRight() + 25);
    section1.setBottom(reverbControls.getBottom() + 5);
    g.drawRoundedRectangle(section1, 15.0f, 3.0f);

    // Section 2: oscillatorControls - ensure bottom is aligned with section1
    juce::Rectangle<float> section2;
    section2.setX(oscillatorControls.getX() - 15);
    section2.setY(oscillatorControls.getY() - 15);
    section2.setRight(oscillatorControls.getRight() + 25);
    section2.setBottom(section1.getBottom()); // Set same bottom as section1
    g.drawRoundedRectangle(section2, 15.0f, 3.0f);

    // Section 3: waveScreen
    juce::Rectangle<float> section3;
    section3.setX(waveScreen.getX() - 15);
    section3.setY(waveScreen.getY() - 15);
    section3.setRight(waveScreen.getRight() + 15);
    section3.setBottom(waveScreen.getBottom() + 15);
    g.drawRoundedRectangle(section3, 15.0f, 3.0f);
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