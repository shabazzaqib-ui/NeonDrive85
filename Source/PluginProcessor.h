#pragma once
#include <JuceHeader.h>
#include "Arpeggiator.h"
//==============================================================================
class SynthSound : public juce::SynthesiserSound
{
public:
    bool appliesToNote (int) override { return true; }
    bool appliesToChannel (int) override { return true; }
};
//==============================================================================
class SynthVoice : public juce::SynthesiserVoice
{
public:
    void setParameterPointers (std::atomic<float>* oscTypeParam,
                                std::atomic<float>* cutoffParam,
                                std::atomic<float>* resonanceParam,
                                std::atomic<float>* attackParam,
                                std::atomic<float>* decayParam,
                                std::atomic<float>* sustainParam,
                                std::atomic<float>* releaseParam)
    {
        oscType = oscTypeParam;
        cutoff = cutoffParam;
        resonance = resonanceParam;
        attack = attackParam;
        decay = decayParam;
        sustain = sustainParam;
        release = releaseParam;
    }
    void prepare (double sampleRate, int samplesPerBlock, int numOutputChannels)
    {
        juce::dsp::ProcessSpec spec { sampleRate, (juce::uint32) samplesPerBlock, (juce::uint32) numOutputChannels };
        filter.prepare (spec);
        filter.setType (juce::dsp::StateVariableTPTFilterType::lowpass);
        adsr.setSampleRate (sampleRate);
    }
    bool canPlaySound (juce::SynthesiserSound* sound) override
    {
        return dynamic_cast<SynthSound*> (sound) != nullptr;
    }
    void startNote (int midiNoteNumber, float velocity,
                     juce::SynthesiserSound*, int) override
    {
        currentFrequency = juce::MidiMessage::getMidiNoteInHertz (midiNoteNumber);
        phase = 0.0;
        level = velocity;
        juce::ADSR::Parameters params;
        params.attack  = attack  ? attack->load()  : 0.01f;
        params.decay   = decay   ? decay->load()   : 0.1f;
        params.sustain = sustain ? sustain->load()  : 0.8f;
        params.release = release ? release->load() : 0.2f;
        adsr.setParameters (params);
        adsr.noteOn();
    }
    void stopNote (float, bool allowTailOff) override
    {
        if (allowTailOff)
            adsr.noteOff();
        else
        {
            clearCurrentNote();
            adsr.reset();
        }
    }
    void pitchWheelMoved (int) override {}
    void controllerMoved (int, int) override {}
    void renderNextBlock (juce::AudioBuffer<float>& outputBuffer,
                           int startSample, int numSamples) override
    {
        if (!isVoiceActive() && !adsr.isActive())
            return;
        int type = oscType ? (int) oscType->load() : 0;
        float cutoffHz = cutoff ? cutoff->load() : 2000.0f;
        float res = resonance ? resonance->load() : 0.2f;
        filter.setCutoffFrequency (cutoffHz);
        filter.setResonance (res);
        double sr = getSampleRate();
        double phaseIncrement = currentFrequency / sr;
        for (int i = 0; i < numSamples; ++i)
        {
            float oscOut = 0.0f;
            if (type == 0)       oscOut = (float) (2.0 * phase - 1.0);       // Saw
            else if (type == 1)  oscOut = phase < 0.5 ? 1.0f : -1.0f;        // Square
            else                 oscOut = phase < 0.25 ? 1.0f : -1.0f;       // Pulse
            phase += phaseIncrement;
            if (phase >= 1.0) phase -= 1.0;
            float filtered = filter.processSample (0, oscOut);
            float env = adsr.getNextSample();
            float sampleOut = filtered * env * level * 0.5f;
            for (int ch = 0; ch < outputBuffer.getNumChannels(); ++ch)
                outputBuffer.addSample (ch, startSample + i, sampleOut);
        }
        if (!adsr.isActive())
            clearCurrentNote();
    }
private:
    double currentFrequency = 440.0;
    double phase = 0.0;
    float level = 1.0f;
    juce::ADSR adsr;
    juce::dsp::StateVariableTPTFilter<float> filter;
    std::atomic<float>* oscType = nullptr;
    std::atomic<float>* cutoff = nullptr;
    std::atomic<float>* resonance = nullptr;
    std::atomic<float>* attack = nullptr;
    std::atomic<float>* decay = nullptr;
    std::atomic<float>* sustain = nullptr;
    std::atomic<float>* release = nullptr;
};
//==============================================================================
class NeonDrive85AudioProcessor : public juce::AudioProcessor
{
public:
    NeonDrive85AudioProcessor();
    ~NeonDrive85AudioProcessor() override;
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }
    const juce::String getName() const override { return "Neon Drive 85"; }
    bool acceptsMidi() const override { return true; }
    bool producesMidi() const override { return false; }
    double getTailLengthSeconds() const override { return 0.0; }
    int getNumPrograms() override { return 1; }
    int getCurrentProgram() override { return 0; }
    void setCurrentProgram (int) override {}
    const juce::String getProgramName (int) override { return {}; }
    void changeProgramName (int, const juce::String&) override {}
    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;
    juce::AudioProcessorValueTreeState apvts;
private:
    juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();
    juce::Synthesiser synth;
    Arpeggiator arp;
    juce::MidiBuffer arpOutputBuffer;
    std::atomic<float>* oscTypeParam = nullptr;
    std::atomic<float>* cutoffParam = nullptr;
    std::atomic<float>* resonanceParam = nullptr;
    std::atomic<float>* attackParam = nullptr;
    std::atomic<float>* decayParam = nullptr;
    std::atomic<float>* sustainParam = nullptr;
    std::atomic<float>* releaseParam = nullptr;
    std::atomic<float>* outputGainParam = nullptr;
    std::atomic<float>* arpOnParam = nullptr;
    std::atomic<float>* arpRateParam = nullptr;
    std::atomic<float>* arpPatternParam = nullptr;
    std::atomic<float>* arpOctavesParam = nullptr;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (NeonDrive85AudioProcessor)
};
