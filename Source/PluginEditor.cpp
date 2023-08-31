/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
DisTorchicAudioProcessorEditor::DisTorchicAudioProcessorEditor (DisTorchicAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p), scopeComponent (audioProcessor.getAudioBufferQueue())
{
    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.
    setSize (400, 400);
    
    addAndMakeVisible (distortionToggle);
    distortionToggle.setTopLeftPosition(100, 70);
    distortionToggle.setSize(50, 50);
    distortionToggle.changeWidthToFitText();
    distortionToggle.addMouseListener (this, true);
    distortionToggle.setToggleState(true, 0);
    
    distortionToggle.onClick = [this] {
        audioProcessor.setDistortionEnabled(distortionToggle.getToggleState());
    };
    
    addAndMakeVisible (scopeComponent);

    auto area = getLocalBounds();
    scopeComponent.setTopLeftPosition (0, 125);
    scopeComponent.setSize (area.getWidth(), area.getHeight() - 175);
}

DisTorchicAudioProcessorEditor::~DisTorchicAudioProcessorEditor()
{
}

//==============================================================================
void DisTorchicAudioProcessorEditor::paint (juce::Graphics& g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll (juce::Colours::red);

    g.setColour (juce::Colours::white);
    g.setFont (45.0f);
    g.drawFittedText ("DisTorchic", 20, 20, 200, 20, juce::Justification::horizontallyCentred, 1);
}

void DisTorchicAudioProcessorEditor::resized()
{
    // This is generally where you'll want to lay out the positions of any
    // subcomponents in your editor..
}
