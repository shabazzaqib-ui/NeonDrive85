#pragma once
#include <JuceHeader.h>
#include "PluginProcessor.h"
class NeonDrive85AudioProcessorEditor : public juce::AudioProcessorEditor
{
public:
    explicit NeonDrive85AudioProcessorEditor (NeonDrive85AudioProcessor& p)
        : AudioProcessorEditor (&p), processor (p)
    {
        setSize (500, 400);
        auto setupSlider = [this] (juce::Slider& s, juce::Label& l, const juce::String& text)
        {
            s.setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
            s.setTextBoxStyle (juce::Slider::TextBoxBelow, false, 60, 16);
            addAndMakeVisible (s);
            l.setText (text, juce::dontSendNotification);
            l.setJustificationType (juce::Justification::centred);
            l.attachToComponent (&s, false);
            addAndMakeVisible (l);
        };
        setupSlider (cutoffSlider, cutoffLabel, "Cutoff");
        setupSlider (resonanceSlider, resonanceLabel, "Resonance");
        setupSlider (attackSlider, attackLabel, "Attack");
        setupSlider (decaySlider, decayLabel, "Decay");
        setupSlider (sustainSlider, sustainLabel, "Sustain");
        setupSlider (releaseSlider, releaseLabel, "Release");
        setupSlider (gainSlider, gainLabel, "Output");
        setupSlider (arpOctavesSlider, arpOctavesLabel, "Octaves");
        oscTypeBox.addItemList ({ "Saw", "Square", "Pulse" }, 1);
        addAndMakeVisible (oscTypeBox);
        arpRateBox.addItemList ({ "1/4", "1/8", "1/16", "1/8T", "1/16T" }, 1);
        addAndMakeVisible (arpRateBox);
        arpPatternBox.addItemList ({ "Up", "Down", "UpDown", "Random" }, 1);
        addAndMakeVisible (arpPatternBox);
        arpOnButton.setButtonText ("Arp On");
        addAndMakeVisible (arpOnButton);
        auto& apvts = processor.apvts;
        cutoffAttachment      = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (apvts, "cutoff", cutoffSlider);
        resonanceAttachment   = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (apvts, "resonance", resonanceSlider);
        attackAttachment      = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (apvts, "attack", attackSlider);
        decayAttachment       = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (apvts, "decay", decaySlider);
        sustainAttachment     = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (apvts, "sustain", sustainSlider);
        releaseAttachment     = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (apvts, "release", releaseSlider);
        gainAttachment        = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (apvts, "outputGain", gainSlider);
        arpOctavesAttachment  = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (apvts, "arpOctaves", arpOctavesSlider);
        oscTypeAttachment     = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment> (apvts, "oscType", oscTypeBox);
        arpRateAttachment     = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment> (apvts, "arpRate", arpRateBox);
        arpPatternAttachment  = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment> (apvts, "arpPattern", arpPatternBox);
        arpOnAttachment       = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment> (apvts, "arpOn", arpOnButton);
    }
    void paint (juce::Graphics& g) override
    {
        g.fillAll (juce::Colour (0xff1a1a2e));
        g.setColour (juce::Colour (0xffff2e97));
        g.setFont (28.0f);
        g.drawFittedText ("NEON DRIVE '85", getLocalBounds().removeFromTop (40),
                           juce::Justification::centred, 1);
    }
    void resized() override
    {
        auto area = getLocalBounds().reduced (20);
        area.removeFromTop (40); // space for title
        auto row1 = area.removeFromTop (100);
        int w = row1.getWidth() / 4;
        cutoffSlider.setBounds     (row1.removeFromLeft (w).reduced (10));
        resonanceSlider.setBounds  (row1.removeFromLeft (w).reduced (10));
        oscTypeBox.setBounds       (row1.removeFromLeft (w).reduced (10, 40));
        gainSlider.setBounds       (row1.removeFromLeft (w).reduced (10));
        auto row2 = area.removeFromTop (100);
        int w2 = row2.getWidth() / 4;
        attackSlider.setBounds  (row2.removeFromLeft (w2).reduced (10));
        decaySlider.setBounds   (row2.removeFromLeft (w2).reduced (10));
        sustainSlider.setBounds (row2.removeFromLeft (w2).reduced (10));
        releaseSlider.setBounds (row2.removeFromLeft (w2).reduced (10));
        area.removeFromTop (20);
        auto row3 = area.removeFromTop (80);
        int w3 = row3.getWidth() / 4;
        arpOnButton.setBounds       (row3.removeFromLeft (w3).reduced (10, 25));
        arpRateBox.setBounds        (row3.removeFromLeft (w3).reduced (10, 25));
        arpPatternBox.setBounds     (row3.removeFromLeft (w3).reduced (10, 25));
        arpOctavesSlider.setBounds  (row3.removeFromLeft (w3).reduced (10));
    }
private:
    NeonDrive85AudioProcessor& processor;
    juce::Slider cutoffSlider, resonanceSlider, attackSlider, decaySlider,
                 sustainSlider, releaseSlider, gainSlider, arpOctavesSlider;
    juce::Label cutoffLabel, resonanceLabel, attackLabel, decayLabel,
                sustainLabel, releaseLabel, gainLabel, arpOctavesLabel;
    juce::ComboBox oscTypeBox, arpRateBox, arpPatternBox;
    juce::ToggleButton arpOnButton;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment>
        cutoffAttachment, resonanceAttachment, attackAttachment, decayAttachment,
        sustainAttachment, releaseAttachment, gainAttachment, arpOctavesAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment>
        oscTypeAttachment, arpRateAttachment, arpPatternAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment>
        arpOnAttachment;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (NeonDrive85AudioProcessorEditor)
};
