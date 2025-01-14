/*
  ==============================================================================

    Knob.cpp
    Created: 9 Jan 2025 5:39:46pm
    Author:  mikey

  ==============================================================================
*/

/*
  ==============================================================================

    Knob.cpp
    Created: 9 Jan 2025 5:39:46pm
    Author:  mikey

  ==============================================================================
*/

#include "Knob.h"

void Knob::drawRotarySlider(juce::Graphics& g, int x, int y, int width, int height,
    float sliderPosProportional, float rotaryStartAngle, float rotaryEndAngle, juce::Slider& slider)
{
    // Get bounds
    auto bounds = juce::Rectangle<int>(x, y, width, height).toFloat();
    auto radius = juce::jmin(bounds.getWidth(), bounds.getHeight()) / 2.0f;

    // Define knob and shape radii
    float knobRadius = radius - 12.0f; // Increased knob size
    float shapeRadius = knobRadius + 8.0f; // Positioned just outside the knob

    auto centerX = bounds.getCentreX();
    auto centerY = bounds.getCentreY();
    auto rx = centerX - knobRadius;
    auto ry = centerY - knobRadius;
    auto rw = knobRadius * 2.0f;
    auto angle = rotaryStartAngle + sliderPosProportional * (rotaryEndAngle - rotaryStartAngle);

    // Draw metallic background
    juce::ColourGradient gradient(juce::Colours::darkgrey, rx, ry,
        juce::Colours::lightgrey, rx + rw, ry + rw, true);
    g.setGradientFill(gradient);
    g.fillEllipse(rx, ry, rw, rw);

    // Draw knob border
    g.setColour(juce::Colours::black);
    g.drawEllipse(rx, ry, rw, rw, 1.0f);

    // Draw pointer
    juce::Path p;
    auto pointerLength = knobRadius * 0.6f;
    auto pointerThickness = 2.0f;
    p.addRectangle(-pointerThickness * 0.5f, -knobRadius, pointerThickness, pointerLength);
    p.applyTransform(juce::AffineTransform::rotation(angle).translated(centerX, centerY));
    g.setColour(juce::Colours::white);
    g.fillPath(p);

    // Oscillator Shapes
    if (slider.getName() == "ShapeKnob")
    {
        // Define angles for the top-right quadrant
        float startAngle = juce::MathConstants<float>::pi / 6.0f;
        float middleAngle = juce::MathConstants<float>::pi * 2.0f / 6.0f; // 0 degrees (vertical)
        float endAngle = juce::MathConstants<float>::pi * 4.0f / 6.0f;

        std::vector<float> positions = { startAngle, middleAngle, endAngle };
        float shapeSize = 3.0f; // Adjusted size

        for (size_t i = 0; i < positions.size(); ++i)
        {
            float shapeAngle = positions[i];
            // Position shapes just outside the knob
            float shapeX = centerX + shapeRadius * std::sin(shapeAngle);
            float shapeY = centerY - shapeRadius * std::cos(shapeAngle);

            g.setColour(juce::Colours::black);
            juce::Path shapePath;

            switch (i)
            {
            case 0: // Sine wave
            {
                shapePath.startNewSubPath(shapeX - shapeSize, shapeY);
                shapePath.cubicTo(shapeX - shapeSize * 0.5f, shapeY - shapeSize,
                    shapeX + shapeSize * 0.5f, shapeY + shapeSize,
                    shapeX + shapeSize, shapeY);
                g.strokePath(shapePath, juce::PathStrokeType(1.0f));
                break;
            }
            case 1: // Triangle wave
            {
                shapePath.startNewSubPath(shapeX - shapeSize, shapeY + shapeSize * 0.5f);
                shapePath.lineTo(shapeX, shapeY - shapeSize * 0.5f);
                shapePath.lineTo(shapeX + shapeSize, shapeY + shapeSize * 0.5f);
                g.strokePath(shapePath, juce::PathStrokeType(1.0f));
                break;
            }
            case 2: // Square wave
            {
                shapePath.startNewSubPath(shapeX - shapeSize, shapeY + shapeSize * 0.5f);
                shapePath.lineTo(shapeX - shapeSize, shapeY - shapeSize * 0.5f);
                shapePath.lineTo(shapeX + shapeSize, shapeY - shapeSize * 0.5f);
                shapePath.lineTo(shapeX + shapeSize, shapeY + shapeSize * 0.5f);
                g.strokePath(shapePath, juce::PathStrokeType(1.0f));
                break;
            }
            }
        }
    }
}

