/*
  ==============================================================================

    WaveScreen.cpp
    Created: 28 Oct 2024 1:09:43pm
    Author:  mikey

  ==============================================================================
*/

#include <JuceHeader.h>
#include "WaveScreen.h"

//==============================================================================
WaveScreen::WaveScreen()
{
    // In your constructor, you should add any child components, and
    // initialise any special settings that your component needs.

}

WaveScreen::~WaveScreen()
{
}

void WaveScreen::paint(juce::Graphics& g)
{
    /* This demo code just fills the component's background and
       draws some placeholder text to get you started.

       You should replace everything in this method with your own
       drawing code..
    */

    g.fillAll(getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId));   // clear the background


    g.setColour(juce::Colours::transparentBlack);
    g.fillRoundedRectangle(getLocalBounds().toFloat(), 10.0f);

    // Draw a thicker outline with rounded edges
    g.setColour(juce::Colours::black);
    g.drawRoundedRectangle(getLocalBounds().toFloat(), 10.0f, 3.0f);   // 10.0f is the corner radius, 3.0f is the line thickness

}

void WaveScreen::resized()
{
    // This method is where you should set the bounds of any child
    // components that your component contains..

}
