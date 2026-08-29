#pragma once
#include <JuceHeader.h>
class Arpeggiator
{
public:
    enum Pattern { Up, Down, UpDown, Random };
    void prepare (double sr) { sampleRate = sr; }
    void setParams (bool on, int rateIndex, int patternIndex, int octaves)
    {
        isOn = on;
        pattern = (Pattern) patternIndex;
        numOctaves = juce::jmax (1, octaves);
        // rateIndex: 0=1/4, 1=1/8, 2=1/16, 3=1/8T, 4=1/16T
        switch (rateIndex)
        {
            case 0: stepFraction = 1.0;        break;
            case 1: stepFraction = 0.5;        break;
            case 2: stepFraction = 0.25;       break;
            case 3: stepFraction = 1.0 / 3.0;  break;
            case 4: stepFraction = 1.0 / 6.0;  break;
            default: stepFraction = 0.25;      break;
        }
    }
    void process (juce::MidiBuffer& incoming, juce::MidiBuffer& outgoing,
                  int numSamples, double bpm)
    {
        if (!isOn)
        {
            outgoing = incoming;
            return;
        }
        // Capture note on/off events, don't pass raw notes through
        for (const auto meta : incoming)
        {
            auto msg = meta.getMessage();
            if (msg.isNoteOn())
                addHeldNote (msg.getNoteNumber());
            else if (msg.isNoteOff())
                removeHeldNote (msg.getNoteNumber());
        }
        if (bpm <= 0.0) bpm = 120.0;
        double samplesPerStep = sampleRate * (60.0 / bpm) * stepFraction;
        for (int i = 0; i < numSamples; ++i)
        {
            if (heldNotes.empty())
            {
                if (currentArpNote >= 0)
                {
                    outgoing.addEvent (juce::MidiMessage::noteOff (1, currentArpNote), i);
                    currentArpNote = -1;
                }
                counter = 0;
                continue;
            }
            if (counter <= 0)
            {
                if (currentArpNote >= 0)
                    outgoing.addEvent (juce::MidiMessage::noteOff (1, currentArpNote), i);
                buildSequenceIfNeeded();
                int note = nextNoteInSequence();
                outgoing.addEvent (juce::MidiMessage::noteOn (1, note, (juce::uint8) 100), i);
                currentArpNote = note;
                counter = (int) samplesPerStep;
            }
            --counter;
        }
    }
private:
    void addHeldNote (int note)
    {
        if (std::find (heldNotes.begin(), heldNotes.end(), note) == heldNotes.end())
            heldNotes.push_back (note);
        sequenceDirty = true;
    }
    void removeHeldNote (int note)
    {
        heldNotes.erase (std::remove (heldNotes.begin(), heldNotes.end(), note), heldNotes.end());
        sequenceDirty = true;
    }
    void buildSequenceIfNeeded()
    {
        if (!sequenceDirty) return;
        sequenceDirty = false;
        sequence.clear();
        std::vector<int> sorted (heldNotes);
        std::sort (sorted.begin(), sorted.end());
        for (int oct = 0; oct < numOctaves; ++oct)
            for (auto n : sorted)
                sequence.push_back (n + oct * 12);
        if (pattern == Down)
            std::reverse (sequence.begin(), sequence.end());
        else if (pattern == UpDown && sequence.size() > 1)
        {
            auto down = sequence;
            std::reverse (down.begin(), down.end());
            sequence.insert (sequence.end(), down.begin() + 1, down.end() - 1);
        }
        seqIndex = 0;
    }
    int nextNoteInSequence()
    {
        if (sequence.empty()) return 60;
        if (pattern == Random)
            return sequence[(size_t) juce::Random::getSystemRandom().nextInt ((int) sequence.size())];
        int note = sequence[(size_t) seqIndex];
        seqIndex = (seqIndex + 1) % (int) sequence.size();
        return note;
    }
    bool isOn = false;
    Pattern pattern = Up;
    int numOctaves = 1;
    double stepFraction = 0.25;
    double sampleRate = 44100.0;
    std::vector<int> heldNotes;
    std::vector<int> sequence;
    bool sequenceDirty = true;
    int seqIndex = 0;
    int counter = 0;
    int currentArpNote = -1;
};
