#include "PluginProcessor.h"
#include "PluginEditor.h"
NeonDrive85AudioProcessor::NeonDrive85AudioProcessor()
    : AudioProcessor (BusesProperties()
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)),
      apvts (*this, nullptr, "PARAMETERS", createParameterLayout())
{
    for (int i = 0; i < 8; ++i)
        synth.addVoice (new SynthVoice());
    synth.addSound (new SynthSound());
    oscTypeParam    = apvts.getRawParameterValue ("oscType");
    cutoffParam     = apvts.getRawParameterValue ("cutoff");
    resonanceParam  = apvts.getRawParameterValue ("resonance");
    attackParam     = apvts.getRawParameterValue ("attack");
    decayParam      = apvts.getRawParameterValue ("decay");
    sustainParam    = apvts.getRawParameterValue ("sustain");
    releaseParam    = apvts.getRawParameterValue ("release");
    outputGainParam = apvts.getRawParameterValue ("outputGain");
    arpOnParam      = apvts.getRawParameterValue ("arpOn");
    arpRateParam    = apvts.getRawParameterValue ("arpRate");
    arpPatternParam = apvts.getRawParameterValue ("arpPattern");
    arpOctavesParam = apvts.getRawParameterValue ("arpOctaves");
    for (int i = 0; i < synth.getNumVoices(); ++i)
    {
        if (auto* v = dynamic_cast<SynthVoice*> (synth.getVoice (i)))
            v->setParameterPointers (oscTypeParam, cutoffParam, resonanceParam,
                                      attackParam, decayParam, sustainParam, releaseParam);
    }
}
NeonDrive85AudioProcessor::~NeonDrive85AudioProcessor() {}
juce::AudioProcessorValueTreeState::ParameterLayout NeonDrive85AudioProcessor::createParameterLayout()
{
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> params;
    params.push_back (std::make_unique<juce::AudioParameterChoice>(
        "oscType", "Oscillator", juce::StringArray { "Saw", "Square", "Pulse" }, 0));
    params.push_back (std::make_unique<juce::AudioParameterFloat>(
        "cutoff", "Filter Cutoff",
        juce::NormalisableRange<float> (20.0f, 20000.0f, 1.0f, 0.3f), 2000.0f));
    params.push_back (std::make_unique<juce::AudioParameterFloat>(
        "resonance", "Filter Resonance", 0.0f, 1.0f, 0.2f));
    params.push_back (std::make_unique<juce::AudioParameterFloat>(
        "attack", "Attack", 0.001f, 3.0f, 0.01f));
    params.push_back (std::make_unique<juce::AudioParameterFloat>(
        "decay", "Decay", 0.001f, 3.0f, 0.1f));
    params.push_back (std::make_unique<juce::AudioParameterFloat>(
        "sustain", "Sustain", 0.0f, 1.0f, 0.8f));
    params.push_back (std::make_unique<juce::AudioParameterFloat>(
        "release", "Release", 0.001f, 5.0f, 0.2f));
    params.push_back (std::make_unique<juce::AudioParameterFloat>(
        "outputGain", "Output Gain", 0.0f, 1.0f, 0.7f));
    params.push_back (std::make_unique<juce::AudioParameterBool>(
        "arpOn", "Arp On/Off", false));
    params.push_back (std::make_unique<juce::AudioParameterChoice>(
        "arpRate", "Arp Rate",
        juce::StringArray { "1/4", "1/8", "1/16", "1/8T", "1/16T" }, 2));
    params.push_back (std::make_unique<juce::AudioParameterChoice>(
        "arpPattern", "Arp Pattern",
        juce::StringArray { "Up", "Down", "UpDown", "Random" }, 0));
    params.push_back (std::make_unique<juce::AudioParameterInt>(
        "arpOctaves", "Arp Octaves", 1, 4, 1));
    return { params.begin(), params.end() };
}
void NeonDrive85AudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    synth.setCurrentPlaybackSampleRate (sampleRate);
    arp.prepare (sampleRate);
    for (int i = 0; i < synth.getNumVoices(); ++i)
    {
        if (auto* v = dynamic_cast<SynthVoice*> (synth.getVoice (i)))
            v->prepare (sampleRate, samplesPerBlock, getTotalNumOutputChannels());
    }
}
void NeonDrive85AudioProcessor::releaseResources() {}
bool NeonDrive85AudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
    return layouts.getMainOutputChannelSet() == juce::AudioChannelSet::stereo();
}
void NeonDrive85AudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    buffer.clear();
    double bpm = 120.0;
    if (auto* playHead = getPlayHead())
    {
        juce::AudioPlayHead::CurrentPositionInfo posInfo;
        if (playHead->getCurrentPosition (posInfo))
            bpm = posInfo.bpm;
    }
    bool arpOn = arpOnParam ? arpOnParam->load() > 0.5f : false;
    int arpRateIndex = arpRateParam ? (int) arpRateParam->load() : 2;
    int arpPatternIndex = arpPatternParam ? (int) arpPatternParam->load() : 0;
    int arpOctaves = arpOctavesParam ? (int) arpOctavesParam->load() : 1;
    arp.setParams (arpOn, arpRateIndex, arpPatternIndex, arpOctaves);
    arpOutputBuffer.clear();
    arp.process (midiMessages, arpOutputBuffer, buffer.getNumSamples(), bpm);
    synth.renderNextBlock (buffer, arpOutputBuffer, 0, buffer.getNumSamples());
    float gain = outputGainParam ? outputGainParam->load() : 0.7f;
    buffer.applyGain (gain);
}
juce::AudioProcessorEditor* NeonDrive85AudioProcessor::createEditor()
{
    return new NeonDrive85AudioProcessorEditor (*this);
}
void NeonDrive85AudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    auto state = apvts.copyState();
    std::unique_ptr<juce::XmlElement> xml (state.createXml());
    copyXmlToBinary (*xml, destData);
}
void NeonDrive85AudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    std::unique_ptr<juce::XmlElement> xmlState (getXmlFromBinary (data, sizeInBytes));
    if (xmlState != nullptr)
        apvts.replaceState (juce::ValueTree::fromXml (*xmlState));
}
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new NeonDrive85AudioProcessor();
}
