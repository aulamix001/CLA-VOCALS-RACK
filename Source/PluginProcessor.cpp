#include "PluginProcessor.h"
#include "PluginEditor.h"
#include <cmath>

juce::AudioProcessorValueTreeState::ParameterLayout VocalsRackProcessor::makeParameters()
{
    juce::AudioProcessorValueTreeState::ParameterLayout p;
    const char* names[] = {"Input Sensitivity", "Bass", "Treble", "Compress", "Reverb", "Delay", "Pitch", "Output", "Balance"};
    for (int i = 0; i < 9; ++i)
    {
        const float limit = i == 8 ? 6.0f : 10.0f;
        p.add(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID(controls[i], 1), names[i],
            juce::NormalisableRange<float>(-limit, limit, 0.1f), 0.0f));
    }
    const juce::StringArray choices[] = {{"Clear", "Sub", "Lower", "Upper"}, {"Clear", "Bite", "Top", "Roof"},
        {"Clear", "Push", "Spank", "Wall"}, {"Clear", "Tight", "Large", "Chamber"},
        {"Clear", "Slap", "Eight", "Quarter"}, {"Clear", "Stereo", "Wide", "Spreader"}};
    for (int i = 0; i < 6; ++i)
        p.add(std::make_unique<juce::AudioParameterChoice>(juce::ParameterID(colours[i], 1), juce::String(names[i + 1]) + " Color", choices[i], 1));
    p.add(std::make_unique<juce::AudioParameterBool>(juce::ParameterID("bypass", 1), "Bypass", false));
    return p;
}
VocalsRackProcessor::VocalsRackProcessor()
    : AudioProcessor(BusesProperties().withInput("Input", juce::AudioChannelSet::stereo(), true)
        .withOutput("Output", juce::AudioChannelSet::stereo(), true)), state(*this, &undo, "VocalsRackState", makeParameters())
{
    for (int i = 0; i < 9; ++i) { values[(size_t)i] = state.getRawParameterValue(controls[i]); faderParameters[(size_t)i] = state.getParameter(controls[i]); }
    for (int i = 0; i < 6; ++i) { modes[(size_t)i] = state.getRawParameterValue(colours[i]); colourParameters[(size_t)i] = state.getParameter(colours[i]); }
    bypassParameter = state.getParameter("bypass");
}
juce::String VocalsRackProcessor::factoryCategoryName(int category)
{
    if (category < 0 || category >= 6) return {};
    if (category == 5) return juce::String::fromUTF8(u8"Voz e viol\u00e3o");
    return RackFactory::categories[(size_t)category];
}
juce::String VocalsRackProcessor::factoryPresetName(int index)
{
    if (index < 0 || index >= 60) return "Inicial / Personalizado";
    const auto& p = RackFactory::presets[(size_t)index];
    return factoryCategoryName(p.category) + " / " + p.name;
}
const juce::String VocalsRackProcessor::getProgramName(int index)
{
    return index == 0 ? "Initial" : factoryPresetName(index - 1);
}
bool VocalsRackProcessor::loadFactoryPreset(int index)
{
    if (index < 0 || index >= 60) return false;
    const auto& preset = RackFactory::presets[(size_t)index];
    // Use the existing parameter attachments: the host and all visible controls see the same values.
    for (size_t i=0;i<9;++i) faderParameters[i]->setValueNotifyingHost(faderParameters[i]->convertTo0to1(preset.faders[i]));
    for (size_t i=0;i<6;++i) colourParameters[i]->setValueNotifyingHost(colourParameters[i]->convertTo0to1((float)preset.colours[i]));
    bypassParameter->setValueNotifyingHost(0);
    selectedFactory.store(index);
    return true;
}
void VocalsRackProcessor::setCurrentProgram(int program)
{
    if (program < 0 || program >= getNumPrograms()) return;
    if (program > 0) { loadFactoryPreset(program - 1); return; }
    for (auto* p : faderParameters) p->setValueNotifyingHost(p->getDefaultValue());
    for (auto* p : colourParameters) p->setValueNotifyingHost(p->getDefaultValue());
    bypassParameter->setValueNotifyingHost(0);
    selectedFactory.store(-1);
}
bool VocalsRackProcessor::isFactoryPresetModified() const
{
    const int index = selectedFactory.load();
    if (index < 0 || index >= 60) return false;
    const auto& preset = RackFactory::presets[(size_t)index];
    for (size_t i=0;i<9;++i) if (std::abs(values[i]->load() - preset.faders[i]) > 0.002f) return true;
    for (size_t i=0;i<6;++i) if ((int)modes[i]->load() != preset.colours[i]) return true;
    return bypassParameter->getValue() > 0.5f;
}
juce::ValueTree VocalsRackProcessor::capturePresetState()
{
    auto snapshot = state.copyState();
    snapshot.setProperty("factoryPreset",selectedFactory.load(),nullptr);
    return snapshot;
}
void VocalsRackProcessor::restorePresetState(const juce::ValueTree& snapshot)
{
    if (!snapshot.hasType("VocalsRackState")) return;
    const int index = snapshot.getProperty("factoryPreset",-1);
    state.replaceState(snapshot);
    selectedFactory.store(index >= 0 && index < 60 ? index : -1);
}
bool VocalsRackProcessor::isBusesLayoutSupported(const BusesLayout& b) const
{
    return b.getMainOutputChannelSet() == juce::AudioChannelSet::stereo()
        && (b.getMainInputChannelSet() == juce::AudioChannelSet::mono() || b.getMainInputChannelSet() == juce::AudioChannelSet::stereo());
}
void VocalsRackProcessor::prepareToPlay(double sr, int)
{
    rate = sr;
    delayLine.setSize(2, (int)std::ceil(rate * 4.1) + 4);
    pitchLine.setSize(2, (int)std::ceil(rate * 0.15) + 4);
    reverb.setSampleRate(rate);
    for (size_t i = 0; i < smooth.size(); ++i) { smooth[i].reset(rate, 0.025); smooth[i].setCurrentAndTargetValue(values[i]->load()); }
    reset();
}
void VocalsRackProcessor::reset()
{
    delayLine.clear(); pitchLine.clear(); reverb.reset();
    tone = {}; envelope = 0; writePosition = pitchPosition = 0; phase = 0;
    for (auto& p : peaks) p.store(0);
    for (auto& c : clips) c.store(false);
}
float VocalsRackProcessor::readDelay(const juce::AudioBuffer<float>& line, int ch, int pos, float samples) const
{
    float at = (float)pos - samples;
    if (at < 0) at += (float)line.getNumSamples();
    const int a = (int)at, b = (a + 1) % line.getNumSamples();
    const float f = at - (float)a;
    return line.getSample(ch, a) * (1 - f) + line.getSample(ch, b) * f;
}
void VocalsRackProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer&)
{
    juce::ScopedNoDenormals noDenormals;
    const int count = buffer.getNumSamples();
    if (count == 0) return;
    if (getTotalNumInputChannels() == 1) buffer.copyFrom(1, 0, buffer, 0, 0, count);
    std::array<float, 4> maxima {};
    const bool bypassed = state.getRawParameterValue("bypass")->load() > 0.5f;
    if (bypassed != wasBypassed) { reset(); wasBypassed = bypassed; }
    std::array<int, 6> mode {};
    for (size_t i = 0; i < 6; ++i) mode[i] = (int)modes[i]->load();
    for (size_t i = 0; i < 9; ++i) smooth[i].setTargetValue(values[i]->load());
    double bpm = 120;
    if (auto* play = getPlayHead()) if (auto position = play->getPosition()) if (auto tempo = position->getBpm()) bpm = *tempo;
    bpm = juce::jlimit(30.0, 300.0, bpm);
    const float beatFraction[] = {0, 0.25f, 0.5f, 1};
    const float delaySamples = (float)(rate * 60 / bpm) * beatFraction[mode[4]];
    const float lowHz[] = {80, 80, 180, 350}, highHz[] = {3000, 3000, 7000, 11000};
    const float lowA = std::exp(-juce::MathConstants<float>::twoPi * lowHz[mode[0]] / (float)rate);
    const float highA = std::exp(-juce::MathConstants<float>::twoPi * highHz[mode[1]] / (float)rate);
    juce::Reverb::Parameters rp;
    rp.roomSize = mode[3] == 1 ? 0.35f : mode[3] == 2 ? 0.8f : 0.6f;
    rp.damping = 0.55f; rp.wetLevel = 1; rp.dryLevel = 0; rp.width = 1; reverb.setParameters(rp);
    auto* left = buffer.getWritePointer(0); auto* right = buffer.getWritePointer(1);
    for (int n = 0; n < count; ++n)
    {
        std::array<float, 9> v {};
        for (size_t i = 0; i < 9; ++i) v[i] = smooth[i].getNextValue();
        float l = left[n], r = right[n];
        if (! bypassed)
        {
            const float inGain = juce::Decibels::decibelsToGain(v[0]);
            l *= inGain; r *= inGain;
        }
        maxima[0] = std::max(maxima[0], std::abs(l)); maxima[1] = std::max(maxima[1], std::abs(r));
        if (! bypassed)
        {
            if (getTotalNumInputChannels() == 2)
            {
                l *= juce::Decibels::decibelsToGain(-std::max(0.0f, v[8]));
                r *= juce::Decibels::decibelsToGain(std::min(0.0f, v[8]));
            }
            float channels[] = {l, r};
            for (size_t ch = 0; ch < 2; ++ch)
            {
                auto& t = tone[ch]; float x = channels[ch];
                t.low = (1 - lowA) * x + lowA * t.low;
                t.high = (1 - highA) * x + highA * t.high;
                if (mode[0]) x += t.low * (juce::Decibels::decibelsToGain(v[1]) - 1);
                if (mode[1]) x += (channels[ch] - t.high) * (juce::Decibels::decibelsToGain(v[2]) - 1);
                channels[ch] = x;
            }
            l = channels[0]; r = channels[1];
            const float peak = std::max(std::abs(l), std::abs(r));
            const float coeff = std::exp(-1.0f / ((float)rate * (peak > envelope ? 0.003f : 0.1f)));
            envelope = coeff * envelope + (1 - coeff) * peak;
            if (mode[2])
            {
                const float threshold = -18 - v[3];
                const float ratio[] = {1, 2, 4, 10};
                const float over = std::max(0.0f, juce::Decibels::gainToDecibels(envelope, -100.0f) - threshold);
                const float gain = juce::Decibels::decibelsToGain(-over * (1 - 1 / ratio[mode[2]]));
                l *= gain; r *= gain;
            }
            float revL = l, revR = r;
            reverb.processStereo(&revL, &revR, 1);
            const float revMix = mode[3] ? juce::Decibels::decibelsToGain(-18 + v[4]) : 0;
            float wet[] = {0, 0};
            for (int ch = 0; ch < 2; ++ch)
            {
                const float dry = ch == 0 ? l : r;
                const float echo = mode[4] ? readDelay(delayLine, ch, writePosition, delaySamples) : 0;
                delayLine.setSample(ch, writePosition, mode[4] ? dry + echo * 0.3f : 0);
                pitchLine.setSample(ch, pitchPosition, dry);
                const float depth = mode[5] == 1 ? 0.001f : mode[5] == 2 ? 0.003f : 0.005f;
                const float ms = 0.02f + depth * (float)std::sin(phase + (ch == 1 ? juce::MathConstants<double>::pi : 0));
                const float chorus = readDelay(pitchLine, ch, pitchPosition, ms * (float)rate);
                wet[ch] = echo * (mode[4] ? juce::Decibels::decibelsToGain(-16 + v[5]) : 0)
                    + chorus * (mode[5] ? juce::Decibels::decibelsToGain(-18 + v[6]) : 0);
            }
            l += revL * revMix + wet[0]; r += revR * revMix + wet[1];
            const float out = juce::Decibels::decibelsToGain(v[7]); l *= out; r *= out;
            writePosition = (writePosition + 1) % delayLine.getNumSamples();
            pitchPosition = (pitchPosition + 1) % pitchLine.getNumSamples();
            phase += juce::MathConstants<double>::twoPi * 0.35 / rate;
            if (phase >= juce::MathConstants<double>::twoPi) phase -= juce::MathConstants<double>::twoPi;
        }
        left[n] = l; right[n] = r;
        maxima[2] = std::max(maxima[2], std::abs(l)); maxima[3] = std::max(maxima[3], std::abs(r));
    }
    for (size_t i = 0; i < 4; ++i)
    {
        peaks[i].store(std::max(maxima[i], peaks[i].load() * (float)std::exp(-count / (rate * 0.3))));
        if (maxima[i] > 1) clips[i].store(true);
    }
}
juce::AudioProcessorParameter* VocalsRackProcessor::getBypassParameter() const { return state.getParameter("bypass"); }
void VocalsRackProcessor::getStateInformation(juce::MemoryBlock& data)
{
    if (auto xml = capturePresetState().createXml()) copyXmlToBinary(*xml, data);
}
void VocalsRackProcessor::setStateInformation(const void* data, int size)
{
    if (auto xml = getXmlFromBinary(data, size)) if (xml->hasTagName("VocalsRackState"))
        restorePresetState(juce::ValueTree::fromXml(*xml));
}
juce::AudioProcessorEditor* VocalsRackProcessor::createEditor() { return new VocalsRackEditor(*this); }
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter() { return new VocalsRackProcessor(); }
