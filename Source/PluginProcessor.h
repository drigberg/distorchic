/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

namespace dsp = juce::dsp;

//==============================================================================
template <typename Type>
class Distortion
{
public:
    //==============================================================================
    Distortion() {
        auto& waveshaper = processorChain.template get<waveshaperIndex>();
        waveshaper.functionToUse = [] (Type x)
                                    {
                                       return juce::jlimit (Type (-0.1), Type (0.1), x); // [6]
//                                        return juce::jmap
//                                        return std::tanh(x);
                                   };
        
        auto& preGain = processorChain.template get<preGainIndex>();   // [5]
        preGain.setGainDecibels (25.0f);                               // [6]
 
        auto& postGain = processorChain.template get<postGainIndex>(); // [7]
        postGain.setGainDecibels (-10.0f);
    }

    //==============================================================================
    void prepare (const juce::dsp::ProcessSpec& spec)
    {
        auto& filter = processorChain.template get<filterIndex>();
        filter.state = FilterCoefs::makeFirstOrderHighPass (spec.sampleRate, 1000.0f);
        processorChain.prepare (spec);
    }

    //==============================================================================
    template <typename ProcessContext>
    void process (const ProcessContext& context) noexcept
    {
        processorChain.process (context);
    }

    //==============================================================================
    void reset() noexcept {
        processorChain.reset();
    }

private:
    //==============================================================================
    enum {
        filterIndex,
        preGainIndex,
        waveshaperIndex,
        postGainIndex
    };
    
    using Filter = juce::dsp::IIR::Filter<Type>;
    using FilterCoefs = juce::dsp::IIR::Coefficients<Type>;
   
    juce::dsp::ProcessorChain<juce::dsp::ProcessorDuplicator<Filter, FilterCoefs>,
                              juce::dsp::Gain<Type>, juce::dsp::WaveShaper<Type>, juce::dsp::Gain<Type>> processorChain;
};

//==============================================================================
/**
*/
class DisTorchicAudioProcessor  : public juce::AudioProcessor
                            #if JucePlugin_Enable_ARA
                             , public juce::AudioProcessorARAExtension
                            #endif
{
public:
    //==============================================================================
    DisTorchicAudioProcessor();
    ~DisTorchicAudioProcessor() override;

    //==============================================================================
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    
    
    void reset() override
    {
        fxChain.reset();
    }
    
    void releaseResources() override;

   #ifndef JucePlugin_PreferredChannelConfigurations
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
   #endif

    void processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer&) override;

    //==============================================================================
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    //==============================================================================
    const juce::String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    //==============================================================================
    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int index) override;
    const juce::String getProgramName (int index) override;
    void changeProgramName (int index, const juce::String& newName) override;

    //==============================================================================
    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;
    
    void setDistortionEnabled (bool enabled);

private:
    //==============================================================================
    
    dsp::ProcessorChain<Distortion<float>> fxChain;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (DisTorchicAudioProcessor)
};
