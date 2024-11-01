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
    setSize (600, 500);
    addAndMakeVisible(keyboardComponent);
	addAndMakeVisible(waveScreen);
	addAndMakeVisible(arpegiattorButton);
	addAndMakeVisible(pitchBendSlider);

	arpegiattorButton.setButtonText("Arp");
	//arpegiattorButton.setToggleState(false, juce::NotificationType::dontSendNotification);

	pitchBendSlider.setSliderStyle(juce::Slider::SliderStyle::LinearBarVertical);

    // 2 Octaves
    int startNote = 60; // C4 (Middle C)
    int endNote = startNote + 48;
    keyboardComponent.setAvailableRange(startNote, endNote);
}

NewProjectAudioProcessorEditor::~NewProjectAudioProcessorEditor()
{
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

    // Reservar espacio a la izquierda para los botones
    auto controlAreaWidth = 100;
    auto controlArea = bounds.removeFromLeft(controlAreaWidth);

    // Bottom elements height
    arpegiattorButton.setBounds(controlArea.removeFromBottom(70));
    pitchBendSlider.setBounds(controlArea.removeFromBottom(10));

    // Keyboard Position
    keyboardComponent.setBounds(bounds.removeFromBottom(130));

    // WaveScreen Position
    auto screenHeight = 130;
    auto screenWidth = 130;
    auto screenX = (getWidth() - screenWidth) / 2;
    auto screenY = keyboardComponent.getY() - screenHeight - 15;
    waveScreen.setBounds(screenX, screenY, screenWidth, screenHeight);
}