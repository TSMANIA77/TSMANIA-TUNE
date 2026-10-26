#include "PluginEditor.h"

namespace { const juce::Colour bg (0xff090707), panel (0xff170b0b), red (0xffff2638), dimRed (0xff8f101c), white (0xfff6eeee); }

TsmaniaTuneAudioProcessorEditor::TsmaniaTuneAudioProcessorEditor (TsmaniaTuneAudioProcessor& p)
    : AudioProcessorEditor (&p), processor (p)
{
    setSize (900, 590);
    auto setupCombo = [this](juce::ComboBox& c) { c.setColour (juce::ComboBox::backgroundColourId, panel); c.setColour (juce::ComboBox::textColourId, white); c.setColour (juce::ComboBox::outlineColourId, dimRed); addAndMakeVisible(c); };
    keyBox.addItemList ({ "C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B" }, 1);
    scaleBox.addItemList ({ "Major", "Minor", "Chromatic" }, 1);
    setupCombo (keyBox); setupCombo (scaleBox);
    auto setupSlider = [this](juce::Slider& s, const juce::String& name) { s.setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag); s.setTextBoxStyle (juce::Slider::TextBoxBelow, false, 72, 22); s.setName(name); s.setColour (juce::Slider::rotarySliderFillColourId, red); s.setColour (juce::Slider::thumbColourId, red); s.setColour (juce::Slider::textBoxTextColourId, white); s.setColour (juce::Slider::textBoxOutlineColourId, dimRed); addAndMakeVisible(s); };
    setupSlider(retune, "RETUNE SPEED"); setupSlider(humanize, "HUMANIZE"); setupSlider(flex, "FLEX-TUNE"); setupSlider(formant, "FORMANT");
    robotic.setColour (juce::ToggleButton::textColourId, white); robotic.setColour (juce::ToggleButton::tickColourId, red); addAndMakeVisible(robotic);
    pitchLabel.setJustificationType (juce::Justification::centred); pitchLabel.setColour (juce::Label::textColourId, red); pitchLabel.setFont (juce::Font (38.0f, juce::Font::bold)); addAndMakeVisible(pitchLabel);
    statusLabel.setJustificationType (juce::Justification::centred); statusLabel.setColour (juce::Label::textColourId, white); addAndMakeVisible(statusLabel);
    keyAttach = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(processor.parameters, "KEY", keyBox);
    scaleAttach = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(processor.parameters, "SCALE", scaleBox);
    retuneAttach = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(processor.parameters, "RETUNE", retune);
    humanizeAttach = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(processor.parameters, "HUMANIZE", humanize);
    flexAttach = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(processor.parameters, "FLEX", flex);
    formantAttach = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(processor.parameters, "FORMANT", formant);
    roboticAttach = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(processor.parameters, "ROBOTIC", robotic);
    startTimerHz (15);
}

void TsmaniaTuneAudioProcessorEditor::paint (juce::Graphics& g)
{
    g.fillAll (bg);
    g.setGradientFill (juce::ColourGradient (juce::Colour (0xff30070d), bg, 0, 0, 900, 590, false));
    g.fillAll (juce::Colour (0x22000000));
    g.setColour (red.withAlpha (0.9f)); g.drawRoundedRectangle (12.0f, 12.0f, 876.0f, 566.0f, 16.0f, 1.5f);
    g.setColour (red); g.setFont (juce::Font (34.0f, juce::Font::bold)); g.drawText ("TSMANIA", 28, 20, 300, 44, juce::Justification::left);
    g.setFont (juce::Font (25.0f, juce::Font::bold)); g.drawText ("TUNE", 31, 59, 160, 30, juce::Justification::left);
    g.setColour (white.withAlpha (0.75f)); g.setFont (12.0f); g.drawText ("VOCAL TUNING ENGINE  |  EXPERIMENTAL DSP 0.2", 31, 88, 340, 20, juce::Justification::left);
    auto panelBox = [&g](juce::Rectangle<int> r, const juce::String& title) { g.setColour(panel); g.fillRoundedRectangle(r.toFloat(), 10.0f); g.setColour(dimRed); g.drawRoundedRectangle(r.toFloat(), 10.0f, 1.0f); g.setColour(white); g.setFont(14.0f); g.drawText(title, r.getX()+12, r.getY()+8, r.getWidth()-24, 22, juce::Justification::left); };
    panelBox ({ 22, 122, 255, 155 }, "KEY / SCALE"); panelBox ({ 292, 122, 316, 250 }, "PITCH TRACKER"); panelBox ({ 623, 122, 255, 155 }, "VOICE MODE"); panelBox ({ 22, 390, 856, 168 }, "CORRECTION CONTROLS");
    g.setColour (red.withAlpha(0.35f)); g.drawEllipse (350.0f, 175.0f, 200.0f, 150.0f, 2.0f);
    g.setColour (white.withAlpha(0.6f)); g.setFont(12.0f); g.drawText("Live pitch estimate", 340, 337, 220, 20, juce::Justification::centred);
    g.setColour (red.withAlpha(0.18f)); for (int i=0;i<14;++i) g.drawVerticalLine (40+i*62, 414.0f, 542.0f);
    g.setColour (white.withAlpha(0.6f)); g.setFont(11.0f); g.drawText("Experimental pitch shifting: verify tuning, artifacts and latency before production use.", 34, 560, 830, 14, juce::Justification::centred);
}

void TsmaniaTuneAudioProcessorEditor::resized()
{
    keyBox.setBounds (38, 165, 220, 32); scaleBox.setBounds (38, 220, 220, 32);
    pitchLabel.setBounds (332, 215, 235, 65); statusLabel.setBounds (335, 282, 230, 28);
    robotic.setBounds (650, 165, 200, 30);
    retune.setBounds (36, 430, 175, 112); humanize.setBounds (235, 430, 175, 112); flex.setBounds (435, 430, 175, 112); formant.setBounds (640, 430, 175, 112);
}

void TsmaniaTuneAudioProcessorEditor::timerCallback()
{
    const float midi = processor.detectedMidi.load();
    if (midi < 0.0f) { pitchLabel.setText ("—", juce::dontSendNotification); statusLabel.setText ("Waiting for voice", juce::dontSendNotification); return; }
    const int note = juce::roundToInt (midi);
    static const char* names[] = { "C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B" };
    pitchLabel.setText (names[(note % 12 + 12) % 12], juce::dontSendNotification);
    const float corrected = processor.correctedMidi.load();
    const int target = corrected >= 0.0f ? juce::roundToInt (corrected) : note;
    statusLabel.setText (juce::String (processor.detectedHz.load(), 1) + " Hz  |  target " + names[(target % 12 + 12) % 12]
                         + "  |  experimental correction", juce::dontSendNotification);
}
