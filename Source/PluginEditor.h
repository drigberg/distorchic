/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"

//==============================================================================
/**
*/
class DisTorchicAudioProcessorEditor  : public juce::AudioProcessorEditor
{
public:
    DisTorchicAudioProcessorEditor (DisTorchicAudioProcessor&);
    ~DisTorchicAudioProcessorEditor() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;

private:
    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    DisTorchicAudioProcessor& audioProcessor;
    
    juce::ToggleButton distortionToggle {"distortion enabled"};

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (DisTorchicAudioProcessorEditor)
};
