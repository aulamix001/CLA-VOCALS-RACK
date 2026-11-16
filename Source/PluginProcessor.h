#pragma once
#include <juce_audio_utils/juce_audio_utils.h>
#include <juce_dsp/juce_dsp.h>
#include <array>
#include "FactoryPresets.h"

class VocalsRackProcessor final : public juce::AudioProcessor
{
public:
    VocalsRackProcessor();
    void prepareToPlay(double, int) override;
    void releaseResources() override {}
    void reset() override;
    bool isBusesLayoutSupported(const BusesLayout&) const override;
    using juce::AudioProcessor::processBlock;
    void processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&) override;
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }
    const juce::String getName() const override { return "Vocals Rack"; }
    bool acceptsMidi() const override { return false; }
    bool producesMidi() const override { return false; }
    double getTailLengthSeconds() const override { return 12.0; }
    int getNumPrograms() override { return 61; } // Initial state plus 60 factory programs.
    int getCurrentProgram() override { return selectedFactory.load() + 1; }
    void setCurrentProgram(int) override;
    const juce::String getProgramName(int) override;
    void changeProgramName(int, const juce::String&) override {}
    void getStateInformation(juce::MemoryBlock&) override;
    void setStateInformation(const void*, int) override;
    juce::AudioProcessorParameter* getBypassParameter() const override;
    bool loadFactoryPreset(int index);
    int getSelectedFactoryPreset() const { return selectedFactory.load(); }
    bool isFactoryPresetModified() const;
    static juce::String factoryCategoryName(int category);
    static juce::String factoryPresetName(int index);
    juce::ValueTree capturePresetState();
    void restorePresetState(const juce::ValueTree&);
    juce::UndoManager undo;
    juce::AudioProcessorValueTreeState state;
    std::array<std::atomic<float>, 4> peaks {};
    std::array<std::atomic<bool>, 4> clips {};
    static constexpr const char* controls[] = {"input", "bass", "treble", "compress", "reverb", "delay", "pitch", "output", "balance"};
    static constexpr const char* colours[] = {"bassColour", "trebleColour", "compressColour", "reverbColour", "delayColour", "pitchColour"};
private:
    static juce::AudioProcessorValueTreeState::ParameterLayout makeParameters();
    std::array<std::atomic<float>*, 9> values {};
    std::array<std::atomic<float>*, 6> modes {};
    std::array<juce::RangedAudioParameter*, 9> faderParameters {};
    std::array<juce::RangedAudioParameter*, 6> colourParameters {};
    juce::RangedAudioParameter* bypassParameter = nullptr;
    std::atomic<int> selectedFactory {-1};
    std::array<juce::SmoothedValue<float>, 9> smooth;
    struct Tone { float low = 0, high = 0; };
    std::array<Tone, 2> tone;
    juce::AudioBuffer<float> delayLine, pitchLine;
    juce::Reverb reverb;
    int writePosition = 0, pitchPosition = 0;
    double rate = 48000, phase = 0;
    float envelope = 0;
    bool wasBypassed = false;
    float readDelay(const juce::AudioBuffer<float>&, int, int, float) const;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(VocalsRackProcessor)
};
