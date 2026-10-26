#pragma once
#include <JuceHeader.h>
#include "PluginProcessor.h"

class TsmaniaTuneAudioProcessorEditor final : public juce::AudioProcessorEditor, private juce::Timer
{
public:
    explicit TsmaniaTuneAudioProcessorEditor (TsmaniaTuneAudioProcessor&);
    ~TsmaniaTuneAudioProcessorEditor() override = default;
    void paint (juce::Graphics&) override;
    void resized() override;
private:
    void timerCallback() override;
    TsmaniaTuneAudioProcessor& processor;
    juce::ComboBox keyBox, scaleBox;
    juce::Slider retune, humanize, flex, formant;
    juce::ToggleButton robotic { "ROBOTIC" };
    juce::Label pitchLabel, statusLabel;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> keyAttach, scaleAttach;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> retuneAttach, humanizeAttach, flexAttach, formantAttach;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> roboticAttach;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (TsmaniaTuneAudioProcessorEditor)
};
