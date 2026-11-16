#include "PluginEditor.h"
#include <BinaryData.h>
namespace {
constexpr float baseW = 1557, baseH = 1010;
constexpr float centres[] = {150, 433, 573, 709, 849, 985, 1123, 1406};
const juce::StringArray labels[] = {{"Clear", "Sub", "Lower", "Upper"}, {"Clear", "Bite", "Top", "Roof"},
    {"Clear", "Push", "Spank", "Wall"}, {"Clear", "Tight", "Large", "Chamber"},
    {"Clear", "Slap", "Eight", "Quarter"}, {"Clear", "Stereo", "Wide", "Spreader"}};
juce::Colour modeColour(int m) { return m == 1 ? juce::Colour(0xffa6e74e) : m == 2 ? juce::Colour(0xff46bbe5) : m == 3 ? juce::Colour(0xfff34a44) : juce::Colours::grey; }
}
void VocalsRackEditor::FaderLook::drawLinearSlider(juce::Graphics& g, int x, int y, int w, int h, float, float, float, juce::Slider::SliderStyle style, juce::Slider& slider)
{
    g.fillAll(juce::Colour(0xff303334));
    if (style != juce::Slider::LinearVertical) return;
    const float cx = (float)x + (float)w * 0.5f;
    g.setColour(juce::Colours::black); g.fillRoundedRectangle(cx - 3, (float)y, 6, (float)h, 3);
    const float kh = (float)slider.getHeight() * 0.24f, kw = (float)w * 0.7f;
    const float pos = (float)y + kh * 0.5f + ((float)h - kh) * (1.0f - (float)slider.valueToProportionOfLength(slider.getValue()));
    juce::Rectangle<float> knob(cx - kw / 2, pos - kh / 2, kw, kh);
    g.setColour(juce::Colours::black.withAlpha(0.6f)); g.fillRoundedRectangle(knob.translated(4, 4), 4);
    g.setGradientFill(juce::ColourGradient(juce::Colour(0xffeeeeee), 0, knob.getY(), juce::Colour(0xff55595c), 0, knob.getCentreY(), false));
    g.fillRoundedRectangle(knob, 3);
    g.setGradientFill(juce::ColourGradient(juce::Colour(0xff55595c), 0, knob.getCentreY(), juce::Colour(0xfff7f9fa), 0, knob.getBottom(), false));
    g.fillRect(knob.withTop(knob.getCentreY()));
    g.setColour(juce::Colour(0xffaaaaaa)); g.drawRoundedRectangle(knob, 3, 1);
    g.setColour(juce::Colour(0xff45484a)); g.drawHorizontalLine((int)knob.getCentreY(), knob.getX() + 4, knob.getRight() - 4);
}
VocalsRackEditor::VocalsRackEditor(VocalsRackProcessor& p) : AudioProcessorEditor(p), processor(p)
{
    reference = juce::ImageCache::getFromMemory(BinaryData::vocalsrackreference_png, BinaryData::vocalsrackreference_pngSize);
    const char* names[] = {"Input Sensitivity", "Bass", "Treble", "Compress", "Reverb", "Delay", "Pitch", "Output", "Balance"};
    for (size_t i = 0; i < sliders.size(); ++i)
    {
        auto& s = sliders[i]; s.setName(names[i]); s.setTitle(names[i]);
        s.setSliderStyle(i == 8 ? juce::Slider::RotaryVerticalDrag : juce::Slider::LinearVertical);
        s.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0); s.setDoubleClickReturnValue(true, 0);
        s.setPopupDisplayEnabled(true, true, this); s.setTextValueSuffix(" dB");
        if (i != 8) s.setLookAndFeel(&look);
        addAndMakeVisible(s);
        attachments[i] = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(p.state, p.controls[i], s);
    }
    for (size_t i = 0; i < modes.size(); ++i)
    {
        auto& b = modes[i]; addAndMakeVisible(b);
        b.onClick = [this, i] {
            auto* parameter = processor.state.getParameter(processor.colours[i]);
            const int next = ((int)processor.state.getRawParameterValue(processor.colours[i])->load() + 1) % 4;
            parameter->beginChangeGesture(); parameter->setValueNotifyingHost(parameter->convertTo0to1((float)next)); parameter->endChangeGesture();
            timerCallback();
        };
    }
    for (auto* b : {&resetButton, &copyButton, &swapButton, &saveButton, &loadButton}) addAndMakeVisible(b);
    addAndMakeVisible(bypassButton);
    bypassAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(p.state, "bypass", bypassButton);
    resetButton.onClick = [this] { for (auto* param : processor.getParameters()) { param->beginChangeGesture(); param->setValueNotifyingHost(param->getDefaultValue()); param->endChangeGesture(); } };
    copyButton.onClick = [this] { alternate = processor.state.copyState(); };
    swapButton.onClick = [this] { if (!alternate.isValid()) alternate = processor.state.copyState(); auto current = processor.state.copyState(); processor.state.replaceState(alternate); alternate = current; isB = !isB; swapButton.setButtonText(isB ? "B / A" : "A / B"); };
    saveButton.onClick = [this] { chooseFile(true); }; loadButton.onClick = [this] { chooseFile(false); };
    setResizable(true, true); getConstrainer()->setFixedAspectRatio(baseW / baseH);
    setResizeLimits(778, 505, 1557, 1010); setSize(1090, 707);
    timerCallback(); startTimerHz(30);
}
VocalsRackEditor::~VocalsRackEditor() { stopTimer(); for (auto& s : sliders) s.setLookAndFeel(nullptr); }
void VocalsRackEditor::chooseFile(bool save)
{
    chooser = std::make_unique<juce::FileChooser>(save ? "Save Vocals Rack preset" : "Load Vocals Rack preset", juce::File{}, "*.vrpreset");
    const auto chooserFlags = juce::FileBrowserComponent::canSelectFiles | (save ? juce::FileBrowserComponent::saveMode | juce::FileBrowserComponent::warnAboutOverwriting : juce::FileBrowserComponent::openMode);
    chooser->launchAsync(chooserFlags, [safe = juce::Component::SafePointer<VocalsRackEditor>(this), save](const juce::FileChooser& fc) {
        if (safe == nullptr) return;
        auto file = fc.getResult(); if (file == juce::File{}) return;
        if (save) { juce::MemoryBlock block; safe->processor.getStateInformation(block); file.withFileExtension("vrpreset").replaceWithData(block.getData(), block.getSize()); }
        else { juce::MemoryBlock block; if (file.loadFileAsData(block)) safe->processor.setStateInformation(block.getData(), (int)block.getSize()); }
    });
}
void VocalsRackEditor::timerCallback()
{
    for (size_t i = 0; i < modes.size(); ++i)
    {
        const int m = (int)processor.state.getRawParameterValue(processor.colours[i])->load();
        modes[i].setButtonText(labels[i][m].toUpperCase() + "  ^");
        modes[i].setColour(juce::TextButton::textColourOffId, modeColour(m));
        modes[i].setColour(juce::TextButton::buttonColourId, juce::Colour(0xff242729));
    }
    sliders[8].setEnabled(processor.getTotalNumInputChannels() == 2);
    repaint();
}
void VocalsRackEditor::resized()
{
    const float sx = (float)getWidth() / baseW, sy = (float)getHeight() / baseH;
    auto bounds = [=](float x, float y, float w, float h) { return juce::Rectangle<int>((int)(x*sx), (int)(y*sy), (int)(w*sx), (int)(h*sy)); };
    for (size_t i = 0; i < 8; ++i) sliders[i].setBounds(bounds(centres[i] - 48, 422, 96, 473));
    sliders[8].setBounds(bounds(245, 838, 55, 55));
    for (size_t i = 0; i < 6; ++i) modes[i].setBounds(bounds(centres[i+1] - 60, 354, 120, 58));
    resetButton.setBounds(bounds(350, 26, 100, 30)); copyButton.setBounds(bounds(900, 26, 120, 30));
    swapButton.setBounds(bounds(1030, 26, 120, 30)); loadButton.setBounds(bounds(1160, 26, 100, 30));
    saveButton.setBounds(bounds(1270, 26, 100, 30)); bypassButton.setBounds(bounds(1380, 26, 130, 30));
}
void VocalsRackEditor::paint(juce::Graphics& g)
{
    g.drawImage(reference, getLocalBounds().toFloat());
    g.addTransform(juce::AffineTransform::scale((float)getWidth()/baseW, (float)getHeight()/baseH));
    g.setColour(juce::Colours::black); g.fillRect(20, 25, 1515, 31);
    g.setColour(juce::Colours::silver); g.setFont(18.0f); g.drawText("VOCALS RACK", 28, 26, 300, 30, juce::Justification::centredLeft);
    // The reference bitmap is immutable. Moving controls and meters are separate layers.
    g.setColour(juce::Colour(0xff242729)); g.fillRect(374, 254, 815, 94);
    for (int i = 0; i < 6; ++i)
    {
        const int selected = (int)processor.state.getRawParameterValue(processor.colours[i])->load();
        for (int m = 1; m <= 3; ++m)
        {
            g.setColour(selected == m ? modeColour(m) : juce::Colour(0xff6e7376)); g.setFont(15.0f);
            g.drawText(labels[i][m].toUpperCase(), (int)centres[i+1]-59, 253 + (3-m)*29, 118, 28, juce::Justification::centred);
        }
    }
    for (int side = 0; side < 2; ++side)
    {
        const float x = side == 0 ? 239.0f : 1256.0f;
        g.setColour(juce::Colour(0xff111718)); g.fillRect(x, 480.0f, 65.0f, 350.0f);
        for (int ch = 0; ch < 2; ++ch)
        {
            const auto index = (size_t)(side*2+ch);
            const float db = juce::Decibels::gainToDecibels(processor.peaks[index].load(), -100.0f);
            const float amount = juce::jlimit(0.0f, 1.0f, (db+26)/26);
            for (int segment = 0; segment < 30; ++segment)
            {
                g.setColour((float)segment / 30 < amount ? juce::Colour(0xff8aee38) : juce::Colour(0xff1d3823));
                g.fillRect(x + (float)ch*44, 819.0f - (float)segment*11, 18.0f, 8.0f);
            }
            g.setColour(processor.clips[index].load() ? juce::Colours::red : juce::Colour(0xff502525));
            g.fillRect(x+(float)ch*44, 483.0f, 18.0f, 8.0f);
        }
    }
    const float inputDb = juce::Decibels::gainToDecibels(std::max(processor.peaks[0].load(), processor.peaks[1].load()), -100.0f);
    g.setColour(inputDb > -1 ? juce::Colours::red : inputDb > -12 ? juce::Colours::yellow : inputDb > -26 ? juce::Colours::green : juce::Colour(0xff333333));
    g.fillRoundedRectangle(137, 428, 27, 17, 3);
}
void VocalsRackEditor::mouseDown(const juce::MouseEvent& e)
{
    const float x = (float)e.x * baseW/(float)getWidth(), y = (float)e.y*baseH/(float)getHeight();
    if (y >= 470 && y <= 835)
        for (int side = 0; side < 2; ++side)
            if (x >= (side == 0 ? 230 : 1240) && x <= (side == 0 ? 315 : 1335))
                for (int ch = 0; ch < 2; ++ch) processor.clips[(size_t)(side*2+ch)].store(false);
}
