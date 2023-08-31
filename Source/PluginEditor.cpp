/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
DisTorchicAudioProcessorEditor::DisTorchicAudioProcessorEditor (DisTorchicAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p)
{
    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.
    setSize (400, 300);
    
    addAndMakeVisible (distortionToggle);
    distortionToggle.setTopLeftPosition(100, 100);
    distortionToggle.setSize(50, 50);
    distortionToggle.changeWidthToFitText();
    distortionToggle.addMouseListener (this, true);
    distortionToggle.setToggleState(true, 0);
    
    distortionToggle.onClick = [this] {
        audioProcessor.setDistortionEnabled(distortionToggle.getToggleState());
    };
}

DisTorchicAudioProcessorEditor::~DisTorchicAudioProcessorEditor()
{
}

//==============================================================================
void DisTorchicAudioProcessorEditor::paint (juce::Graphics& g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));

    g.setColour (juce::Colours::white);
    g.setFont (15.0f);
    g.drawFittedText ("Hello World!", getLocalBounds(), juce::Justification::centred, 1);
}

void DisTorchicAudioProcessorEditor::resized()
{
    // This is generally where you'll want to lay out the positions of any
    // subcomponents in your editor..
}
