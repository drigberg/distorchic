/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
DisTorchicAudioProcessor::DisTorchicAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
     : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                     #endif
                       )
#endif
{
}

DisTorchicAudioProcessor::~DisTorchicAudioProcessor()
{
}

//==============================================================================
const juce::String DisTorchicAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool DisTorchicAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool DisTorchicAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool DisTorchicAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double DisTorchicAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int DisTorchicAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int DisTorchicAudioProcessor::getCurrentProgram()
{
    return 0;
}

void DisTorchicAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String DisTorchicAudioProcessor::getProgramName (int index)
{
    return {};
}

void DisTorchicAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void DisTorchicAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    dsp::ProcessSpec spec { sampleRate, static_cast<juce::uint32> (samplesPerBlock), 2 };
    fxChain.prepare (spec);
}

void DisTorchicAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool DisTorchicAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
  #if JucePlugin_IsMidiEffect
    juce::ignoreUnused (layouts);
    return true;
  #else
    // This is the place where you check if the layout is supported.
    // In this template code we only support mono or stereo.
    // Some plugin hosts, such as certain GarageBand versions, will only
    // load plugins that support stereo bus layouts.
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
     && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    // This checks if the input layout matches the output layout
   #if ! JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
   #endif

    return true;
  #endif
}
#endif


void DisTorchicAudioProcessor::reset () {
    fxChain.reset();
}

void DisTorchicAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    // In case we have more outputs than inputs, this code clears any output
    // channels that didn't contain input data, (because these aren't
    // guaranteed to be empty - they may contain garbage).
    // This is here to avoid people getting screaming feedback
    // when they first compile a plugin, but obviously you don't need to keep
    // this code if your algorithm always overwrites all the output channels.
    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());

    // This is the place where you'd normally do the guts of your plugin's
    // audio processing...
    // Make sure to reset the state if your inner loop is processing
    // the samples and the outer loop is handling the channels.
    // Alternatively, you can process the samples with the channels
    // interleaved by keeping the same state.
    for (int channel = 0; channel < totalNumInputChannels; ++channel)
    {
        auto* channelData = buffer.getWritePointer (channel);
        juce::ignoreUnused (channelData);
    }
        
    auto block = dsp::AudioBlock<float> (buffer);
    auto context = dsp::ProcessContextReplacing<float> (block);
    fxChain.process(context);
//    auto outputBlock = context.getOutputBlock();
//    scopeDataCollector.process(outputBlock.getChannelPointer(0),  (size_t) outputBlock.getNumSamples());
    scopeDataCollector.process (buffer.getReadPointer(0), (size_t) buffer.getNumSamples());

}

//==============================================================================
bool DisTorchicAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* DisTorchicAudioProcessor::createEditor()
{
    return new DisTorchicAudioProcessorEditor (*this);
}

//==============================================================================
void DisTorchicAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
}

void DisTorchicAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
}

void DisTorchicAudioProcessor::setDistortionEnabled (bool enabled) {
    fxChain.setBypassed<0>(!enabled);
}

void DisTorchicAudioProcessor::setPreGain(float value) {
    auto& distortion = fxChain.template get<0>();
    distortion.setPreGain(value);
}

void DisTorchicAudioProcessor::setPostGain(float value) {
    auto& distortion = fxChain.template get<0>();
    distortion.setPostGain(value);
}


AudioBufferQueue<float>& DisTorchicAudioProcessor::getAudioBufferQueue() noexcept {
    return audioBufferQueue;
}


//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new DisTorchicAudioProcessor();
}
