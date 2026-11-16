#pragma once
#include "PluginProcessor.h"
class VocalsRackEditor final : public juce::AudioProcessorEditor, private juce::Timer
{
public:
    explicit VocalsRackEditor(VocalsRackProcessor&);
    ~VocalsRackEditor() override;
    void paint(juce::Graphics&) override;
    void resized() override;
    void mouseDown(const juce::MouseEvent&) override;
private:
    struct FaderLook : juce::LookAndFeel_V4
    {
        void drawLinearSlider(juce::Graphics&, int, int, int, int, float, float, float, juce::Slider::SliderStyle, juce::Slider&) override;
        juce::Slider::SliderLayout getSliderLayout(juce::Slider&) override;
        void drawRotarySlider(juce::Graphics&, int, int, int, int, float, float, float, juce::Slider&) override;
        void drawButtonBackground(juce::Graphics&, juce::Button&, const juce::Colour&, bool, bool) override;
        void drawButtonText(juce::Graphics&, juce::TextButton&, bool, bool) override;
        void drawToggleButton(juce::Graphics&, juce::ToggleButton&, bool, bool) override;
        void drawComboBox(juce::Graphics&, int, int, bool, int, int, int, int, juce::ComboBox&) override;
        juce::Font getComboBoxFont(juce::ComboBox&) override;
        void positionComboBoxText(juce::ComboBox&, juce::Label&) override;
    } look;
    void timerCallback() override;
    void chooseFile(bool save);
    void buildPanel();
    VocalsRackProcessor& processor;
    juce::Image reference;
    juce::Image panel;
    std::array<juce::Slider, 9> sliders;
    std::array<std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment>, 9> attachments;
    std::array<juce::TextButton, 6> modes;
    juce::ComboBox presetBox;
    juce::TextButton previousPreset {"<"}, nextPreset {">"};
    juce::TextButton resetButton {"Reset"}, copyButton {"A > B"}, swapButton {"A / B"}, saveButton {"Save"}, loadButton {"Load"};
    juce::ToggleButton bypassButton {"Bypass"};
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> bypassAttachment;
    juce::ValueTree alternate;
    std::unique_ptr<juce::FileChooser> chooser;
    bool isB = false;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(VocalsRackEditor)
};
