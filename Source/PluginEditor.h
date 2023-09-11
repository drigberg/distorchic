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
class DisTorchicAudioProcessorEditor  : public juce::AudioProcessorEditor, public juce::Slider::Listener
{
public:
    DisTorchicAudioProcessorEditor (DisTorchicAudioProcessor&);
    ~DisTorchicAudioProcessorEditor() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;
    void sliderValueChanged (juce::Slider* slider) override;


private:
    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    DisTorchicAudioProcessor& audioProcessor;
    
    ScopeComponent<float> scopeComponent;
    juce::ToggleButton distortionToggle {"distortion enabled"};
    juce::Slider preGainSlider;
    juce::Label  preGainSliderLabel;
    juce::Slider postGainSlider;
    juce::Label  postGainSliderLabel;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (DisTorchicAudioProcessorEditor)
};
