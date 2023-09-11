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
                                       return juce::jlimit (Type (-0.1), Type (0.1), x);
                                       // return std::tanh(x);
                                   };
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
    
    void setPreGain(float value) {
        auto& preGain = processorChain.template get<preGainIndex>();
        preGain.setGainDecibels (value);
    }
    
    void setPostGain(float value) {
        auto& postGain = processorChain.template get<postGainIndex>();
        postGain.setGainDecibels (value);
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
template <typename SampleType>
class AudioBufferQueue
{
public:
    //==============================================================================
    static constexpr size_t order = 9;
    static constexpr size_t bufferSize = 1U << order;
    static constexpr size_t numBuffers = 5;

    //==============================================================================
    void push (const SampleType* dataToPush, size_t numSamples)
    {
        jassert (numSamples <= bufferSize);

        int start1, size1, start2, size2;
        abstractFifo.prepareToWrite (1, start1, size1, start2, size2);

        jassert (size1 <= 1);
        jassert (size2 == 0);

        if (size1 > 0)
            juce::FloatVectorOperations::copy (buffers[(size_t) start1].data(), dataToPush, (int) juce::jmin (bufferSize, numSamples));

        abstractFifo.finishedWrite (size1);
    }

    //==============================================================================
    void pop (SampleType* outputBuffer)
    {
        int start1, size1, start2, size2;
        abstractFifo.prepareToRead (1, start1, size1, start2, size2);

        jassert (size1 <= 1);
        jassert (size2 == 0);

        if (size1 > 0)
            juce::FloatVectorOperations::copy (outputBuffer, buffers[(size_t) start1].data(), (int) bufferSize);

        abstractFifo.finishedRead (size1);
    }

private:
    //==============================================================================
    juce::AbstractFifo abstractFifo { numBuffers };
    std::array<std::array<SampleType, bufferSize>, numBuffers> buffers;
};


//==============================================================================
template <typename SampleType>
class ScopeDataCollector
{
public:
    //==============================================================================
    ScopeDataCollector (AudioBufferQueue<SampleType>& queueToUse)
        : audioBufferQueue (queueToUse)
    {}

    //==============================================================================
    void process (const SampleType* data, size_t numSamples)
    {
        size_t index = 0;

        if (state == State::waitingForTrigger)
        {
            while (index++ < numSamples)
            {
                auto currentSample = *data++;

                if (currentSample >= triggerLevel && prevSample < triggerLevel)
                {
                    numCollected = 0;
                    state = State::collecting;
                    break;
                }

                prevSample = currentSample;
            }
        }

        if (state == State::collecting)
        {
            while (index++ < numSamples)
            {
                buffer[numCollected++] = *data++;

                if (numCollected == buffer.size())
                {
                    audioBufferQueue.push (buffer.data(), buffer.size());
                    state = State::waitingForTrigger;
                    prevSample = SampleType (100);
                    break;
                }
            }
        }
    }

private:
    //==============================================================================
    AudioBufferQueue<SampleType>& audioBufferQueue;
    std::array<SampleType, AudioBufferQueue<SampleType>::bufferSize> buffer;
    size_t numCollected;
    SampleType prevSample = SampleType (100);

    static constexpr auto triggerLevel = SampleType (0.05);

    enum class State { waitingForTrigger, collecting } state { State::waitingForTrigger };
};

//==============================================================================
template <typename SampleType>
class ScopeComponent  : public juce::Component,
                        private juce::Timer
{
public:
    using Queue = AudioBufferQueue<SampleType>;

    //==============================================================================
    ScopeComponent (Queue& queueToUse)
        : audioBufferQueue (queueToUse)
    {
        sampleData.fill (SampleType (0));
        setFramesPerSecond (30);
    }

    //==============================================================================
    void setFramesPerSecond (int framesPerSecond)
    {
        jassert (framesPerSecond > 0 && framesPerSecond < 1000);
        startTimerHz (framesPerSecond);
    }

    //==============================================================================
    void paint (juce::Graphics& g) override
    {
        g.fillAll (juce::Colours::black);
        g.setColour (juce::Colours::white);

        auto area = getLocalBounds();
        auto h = (SampleType) area.getHeight();
        auto w = (SampleType) area.getWidth();

        // Oscilloscope
        auto scopeRect = juce::Rectangle<SampleType> { SampleType (0), SampleType (0), w, h / 2 };
        plot (sampleData.data(), sampleData.size(), g, scopeRect, SampleType (1), h / 4);

        // Spectrum
        auto spectrumRect = juce::Rectangle<SampleType> { SampleType (0), h / 2, w, h / 2 };
        plot (spectrumData.data(), spectrumData.size() / 4, g, spectrumRect);
    }

    //==============================================================================
    void resized() override {}

private:
    //==============================================================================
    Queue& audioBufferQueue;
    std::array<SampleType, Queue::bufferSize> sampleData;

    juce::dsp::FFT fft { Queue::order };
    using WindowFun = juce::dsp::WindowingFunction<SampleType>;
    WindowFun windowFun { (size_t) fft.getSize(), WindowFun::hann };
    std::array<SampleType, 2 * Queue::bufferSize> spectrumData;

    //==============================================================================
    void timerCallback() override
    {
        audioBufferQueue.pop (sampleData.data());
        juce::FloatVectorOperations::copy (spectrumData.data(), sampleData.data(), (int) sampleData.size());

        auto fftSize = (size_t) fft.getSize();

        jassert (spectrumData.size() == 2 * fftSize);
        windowFun.multiplyWithWindowingTable (spectrumData.data(), fftSize);
        fft.performFrequencyOnlyForwardTransform (spectrumData.data());

        static constexpr auto mindB = SampleType (-160);
        static constexpr auto maxdB = SampleType (0);

        for (auto& s : spectrumData)
            s = juce::jmap (juce::jlimit (mindB, maxdB, juce::Decibels::gainToDecibels (s) - juce::Decibels::gainToDecibels (SampleType (fftSize))), mindB, maxdB, SampleType (0), SampleType (1));

        repaint();
    }

    //==============================================================================
    static void plot (const SampleType* data,
                      size_t numSamples,
                      juce::Graphics& g,
                      juce::Rectangle<SampleType> rect,
                      SampleType scaler = SampleType (1),
                      SampleType offset = SampleType (0))
    {
        auto w = rect.getWidth();
        auto h = rect.getHeight();
        auto right = rect.getRight();

        auto center = rect.getBottom() - offset;
        auto gain = h * scaler;

        for (size_t i = 1; i < numSamples; ++i)
            g.drawLine ({ juce::jmap (SampleType (i - 1), SampleType (0), SampleType (numSamples - 1), SampleType (right - w), SampleType (right)),
                          center - gain * data[i - 1],
                          juce::jmap (SampleType (i), SampleType (0), SampleType (numSamples - 1), SampleType (right - w), SampleType (right)),
                          center - gain * data[i] });
    }
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
    
    void reset() override;
    
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
    void setPreGain(float value);
    void setPostGain(float value);
    
    AudioBufferQueue<float>& getAudioBufferQueue() noexcept;


private:
    //==============================================================================
    
    dsp::ProcessorChain<Distortion<float>> fxChain;

    AudioBufferQueue<float> audioBufferQueue;
    ScopeDataCollector<float> scopeDataCollector { audioBufferQueue };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (DisTorchicAudioProcessor)
};
