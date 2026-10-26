#pragma once
#include <JuceHeader.h>
#include <juce_dsp/juce_dsp.h>

class TsmaniaTuneAudioProcessor final : public juce::AudioProcessor
{
public:
    TsmaniaTuneAudioProcessor();
    ~TsmaniaTuneAudioProcessor() override = default;

    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }
    const juce::String getName() const override { return JucePlugin_Name; }
    bool acceptsMidi() const override { return false; }
    bool producesMidi() const override { return false; }
    bool isMidiEffect() const override { return false; }
    double getTailLengthSeconds() const override { return 0.0; }
    int getNumPrograms() override { return 1; }
    int getCurrentProgram() override { return 0; }
    void setCurrentProgram (int) override {}
    const juce::String getProgramName (int) override { return {}; }
    void changeProgramName (int, const juce::String&) override {}
    void getStateInformation (juce::MemoryBlock&) override;
    void setStateInformation (const void*, int) override;

    juce::AudioProcessorValueTreeState parameters;
    std::atomic<float> detectedHz { 0.0f };
    std::atomic<float> detectedMidi { -1.0f };
    std::atomic<float> correctedMidi { -1.0f };
    std::atomic<float> inputLevel { 0.0f };
    double currentSampleRate = 44100.0;

    static juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();

private:
    float estimatePitch (const float* samples, int numSamples, double sampleRate) const;
    float targetMidiNote (float midi, int key, int scale) const;
    juce::dsp::PitchShifter<float> pitchShifters[2];
    juce::AudioBuffer<float> pitchHistory;
    int historyWrite = 0;
    int historyFilled = 0;
    float smoothedShiftSemitones = 0.0f;
    float lastTargetMidi = -1.0f;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (TsmaniaTuneAudioProcessor)
};
