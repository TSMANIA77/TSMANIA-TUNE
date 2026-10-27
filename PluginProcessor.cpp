#include "PluginProcessor.h"
#include "PluginEditor.h"
#include <array>

TsmaniaTuneAudioProcessor::TsmaniaTuneAudioProcessor()
    : AudioProcessor (BusesProperties().withInput ("Input", juce::AudioChannelSet::stereo(), true)
                                       .withOutput("Output", juce::AudioChannelSet::stereo(), true)),
      parameters (*this, nullptr, "PARAMETERS", createParameterLayout())
{}

juce::AudioProcessorValueTreeState::ParameterLayout TsmaniaTuneAudioProcessor::createParameterLayout()
{
    using namespace juce;
    std::vector<std::unique_ptr<RangedAudioParameter>> p;
    p.push_back (std::make_unique<AudioParameterChoice>("KEY", "Key",
        StringArray { "C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B" }, 0));
    p.push_back (std::make_unique<AudioParameterChoice>("SCALE", "Scale",
        StringArray { "Major", "Minor", "Chromatic" }, 1));
    p.push_back (std::make_unique<AudioParameterFloat>("RETUNE", "Retune Speed", NormalisableRange<float>(0.0f, 100.0f, 0.1f), 10.0f));
    p.push_back (std::make_unique<AudioParameterFloat>("HUMANIZE", "Humanize", NormalisableRange<float>(0.0f, 100.0f, 0.1f), 25.0f));
    p.push_back (std::make_unique<AudioParameterFloat>("FLEX", "Flex-Tune", NormalisableRange<float>(0.0f, 100.0f, 0.1f), 20.0f));
    p.push_back (std::make_unique<AudioParameterFloat>("FORMANT", "Formant", NormalisableRange<float>(-12.0f, 12.0f, 0.1f), 0.0f));
    p.push_back (std::make_unique<AudioParameterBool>("ROBOTIC", "Robotic Mode", false));
    return { p.begin(), p.end() };
}

void TsmaniaTuneAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    currentSampleRate = sampleRate;
    juce::dsp::ProcessSpec spec { sampleRate, (juce::uint32) juce::jmax (samplesPerBlock, 512), (juce::uint32) getTotalNumOutputChannels() };
    for (auto& shifter : pitchShifters)
    {
        shifter.reset();
        shifter.prepare (spec);
        shifter.setPitchFactor (1.0f);
    }
    pitchHistory.setSize (1, 4096);
    pitchHistory.clear();
    historyWrite = historyFilled = 0;
    smoothedShiftSemitones = 0.0f;
    lastTargetMidi = -1.0f;
    detectedHz.store (0.0f);
    detectedMidi.store (-1.0f);
    correctedMidi.store (-1.0f);
}

void TsmaniaTuneAudioProcessor::releaseResources()
{
    for (auto& shifter : pitchShifters) shifter.reset();
}

bool TsmaniaTuneAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
    const auto in = layouts.getMainInputChannelSet();
    const auto out = layouts.getMainOutputChannelSet();
    return in == out && (out == juce::AudioChannelSet::mono() || out == juce::AudioChannelSet::stereo());
}

float TsmaniaTuneAudioProcessor::estimatePitch (const float* x, int n, double sr) const
{
    // Difference-based normalized autocorrelation, intended for monophonic voice.
    if (n < 1024 || sr <= 0.0) return 0.0f;
    const int minLag = juce::jmax (2, (int) (sr / 900.0));
    const int maxLag = juce::jmin (n / 2, (int) (sr / 70.0));
    double mean = 0.0;
    for (int i = 0; i < n; ++i) mean += x[i];
    mean /= n;
    double energy = 0.0;
    for (int i = 0; i < n; ++i) { const double v = x[i] - mean; energy += v * v; }
    if (energy / n < 1.0e-7) return 0.0f;

    double bestScore = -1.0;
    int bestLag = 0;
    for (int lag = minLag; lag <= maxLag; ++lag)
    {
        double cross = 0.0, e1 = 0.0, e2 = 0.0;
        for (int i = 0; i < n - lag; i += 2)
        {
            const double a = x[i] - mean, b = x[i + lag] - mean;
            cross += a * b; e1 += a * a; e2 += b * b;
        }
        const double score = cross / (std::sqrt (e1 * e2) + 1.0e-12);
        if (score > bestScore) { bestScore = score; bestLag = lag; }
    }
    if (bestLag == 0 || bestScore < 0.35) return 0.0f;
    return (float) (sr / bestLag);
}

float TsmaniaTuneAudioProcessor::targetMidiNote (float midi, int key, int scale) const
{
    if (scale == 2) return std::round (midi); // chromatic scale
    static const int major[] = { 0, 2, 4, 5, 7, 9, 11 };
    static const int minor[] = { 0, 2, 3, 5, 7, 8, 10 };
    const int* intervals = (scale == 0 ? major : minor);
    float bestNote = midi;
    float bestDistance = 1000.0f;
    const int base = (int) std::floor (midi);
    for (int note = base - 12; note <= base + 12; ++note)
    {
        int pc = (note % 12 + 12) % 12;
        int rel = (pc - key + 12) % 12;
        bool allowed = false;
        for (int i = 0; i < 7; ++i) if (intervals[i] == rel) allowed = true;
        if (allowed && std::abs (midi - (float) note) < bestDistance)
        {
            bestDistance = std::abs (midi - (float) note);
            bestNote = (float) note;
        }
    }
    return bestNote;
}

void TsmaniaTuneAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer&)
{
    juce::ScopedNoDenormals noDenormals;
    const int inputChannels = getTotalNumInputChannels();
    const int outputChannels = getTotalNumOutputChannels();
    const int samples = buffer.getNumSamples();
    for (int ch = inputChannels; ch < outputChannels; ++ch) buffer.clear (ch, 0, samples);
    if (inputChannels == 0 || samples == 0) return;

    float level = 0.0f;
    for (int ch = 0; ch < inputChannels; ++ch)
        level = juce::jmax (level, buffer.getMagnitude (ch, 0, samples));
    inputLevel.store (level);

    // Maintain a rolling history so pitch detection works with small DAW buffers.
    const float* mono = buffer.getReadPointer (0);
    for (int i = 0; i < samples; ++i)
    {
        pitchHistory.setSample (0, historyWrite, mono[i]);
        historyWrite = (historyWrite + 1) % pitchHistory.getNumSamples();
        historyFilled = juce::jmin (historyFilled + 1, pitchHistory.getNumSamples());
    }
    float hz = 0.0f;
    if (historyFilled >= 2048)
    {
        std::array<float, 4096> window {};
        const int start = (historyWrite - historyFilled + pitchHistory.getNumSamples()) % pitchHistory.getNumSamples();
        for (int i = 0; i < historyFilled; ++i)
            window[(size_t) i] = pitchHistory.getSample (0, (start + i) % pitchHistory.getNumSamples());
        hz = estimatePitch (window.data(), historyFilled, currentSampleRate);
    }
    detectedHz.store (hz);
    const float midi = hz > 0.0f ? 69.0f + 12.0f * std::log2 (hz / 440.0f) : -1.0f;
    detectedMidi.store (midi);

    const int key = (int) parameters.getRawParameterValue ("KEY")->load();
    const int scale = (int) parameters.getRawParameterValue ("SCALE")->load();
    const float retune = parameters.getRawParameterValue ("RETUNE")->load();
    const float humanize = parameters.getRawParameterValue ("HUMANIZE")->load();
    const float flex = parameters.getRawParameterValue ("FLEX")->load();
    const bool robotic = parameters.getRawParameterValue ("ROBOTIC")->load() > 0.5f;

    float shift = 0.0f;
    if (midi >= 0.0f)
    {
        const float target = targetMidiNote (midi, key, scale);
        correctedMidi.store (target);
        const float error = target - midi;
        // Flex-Tune creates a small no-correction zone around the target note.
        const float deadZone = (flex / 100.0f) * 0.45f;
        const float effectiveError = std::abs (error) <= deadZone ? 0.0f : error;
        const float amount = robotic ? 1.0f : juce::jlimit (0.15f, 1.0f, 1.0f - humanize / 125.0f);
        shift = effectiveError * amount;
        // Retune speed controls smoothing: zero is fastest, high values are slower.
        const float smoothing = robotic ? 0.0f : juce::jlimit (0.0f, 0.96f, retune / 110.0f);
        smoothedShiftSemitones = smoothing * smoothedShiftSemitones + (1.0f - smoothing) * shift;
        lastTargetMidi = target;
    }
    else
    {
        correctedMidi.store (-1.0f);
        smoothedShiftSemitones *= 0.98f;
    }

    const float factor = std::pow (2.0f, smoothedShiftSemitones / 12.0f);
    for (int ch = 0; ch < juce::jmin (inputChannels, 2); ++ch)
    {
        pitchShifters[ch].setPitchFactor (juce::jlimit (0.5f, 2.0f, factor));
        juce::dsp::AudioBlock<float> block (buffer);
        auto channelBlock = block.getSingleChannelBlock ((size_t) ch);
        juce::dsp::ProcessContextReplacing<float> context (channelBlock);
        pitchShifters[ch].process (context);
    }
}

juce::AudioProcessorEditor* TsmaniaTuneAudioProcessor::createEditor() { return new TsmaniaTuneAudioProcessorEditor (*this); }
void TsmaniaTuneAudioProcessor::getStateInformation (juce::MemoryBlock& dest)
{
    auto state = parameters.copyState();
    std::unique_ptr<juce::XmlElement> xml (state.createXml());
    copyXmlToBinary (*xml, dest);
}
void TsmaniaTuneAudioProcessor::setStateInformation (const void* data, int size)
{
    std::unique_ptr<juce::XmlElement> xml (getXmlFromBinary (data, size));
    if (xml != nullptr && xml->hasTagName (parameters.state.getType()))
        parameters.replaceState (juce::ValueTree::fromXml (*xml));
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter() { return new TsmaniaTuneAudioProcessor(); }
