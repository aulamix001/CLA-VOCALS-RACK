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
namespace {
juce::Colour metal(unsigned int c) { return juce::Colour(c); }
void screw(juce::Graphics& g, float x, float y)
{
    g.setColour(juce::Colours::black.withAlpha(0.8f)); g.fillEllipse(x-12, y-11, 24, 24);
    g.setGradientFill(juce::ColourGradient(metal(0xffb09a74), x-9, y-9, metal(0xff171a1b), x+9, y+10, false));
    g.fillEllipse(x-10,y-10,20,20);
    g.setColour(metal(0xff17191a)); g.fillEllipse(x-7,y-7,14,14);
    g.setColour(metal(0xff847355)); g.drawEllipse(x-4,y-4,8,8,1.5f);
    g.setColour(juce::Colours::black); g.fillEllipse(x-2,y-2,4,4);
}
void bezel(juce::Graphics& g, juce::Rectangle<float> r)
{
    g.setColour(juce::Colours::black); g.fillRect(r.expanded(3));
    g.setGradientFill(juce::ColourGradient(metal(0xffa58c61),r.getX(),r.getY(),metal(0xff131615),r.getRight(),r.getBottom(),false));
    g.fillRect(r);
    g.setColour(metal(0xff111515)); g.fillRect(r.reduced(3));
    g.setColour(metal(0xff6b5940)); g.drawRect(r.reduced(4),0.6f);
}
}
juce::Slider::SliderLayout VocalsRackEditor::FaderLook::getSliderLayout(juce::Slider& s)
{
    juce::Slider::SliderLayout layout;
    auto r=s.getLocalBounds();
    if (s.getSliderStyle()==juce::Slider::LinearVertical)
        layout.sliderBounds=r.withTrimmedTop(juce::roundToInt((float)r.getHeight()*0.136f)).withTrimmedBottom(juce::roundToInt((float)r.getHeight()*0.136f));
    else layout.sliderBounds=r;
    return layout;
}
void VocalsRackEditor::FaderLook::drawLinearSlider(juce::Graphics& g, int, int, int, int, float pos, float, float, juce::Slider::SliderStyle style, juce::Slider& slider)
{
    if (style!=juce::Slider::LinearVertical) return;
    const float scale=(float)slider.getHeight()/440.0f;
    const float cx=(float)slider.getWidth()*0.5f, w=64*scale,h=113*scale;
    const float l=cx-w*0.5f,r=cx+w*0.5f,t=pos-h*0.5f,b=pos+h*0.5f;
    juce::Path face;
    face.startNewSubPath(l+5*scale,t);
    face.lineTo(r-6*scale,t);
    face.cubicTo(r+2*scale,t+25*scale,r+3*scale,b-28*scale,r-4*scale,b);
    face.lineTo(l+3*scale,b);
    face.cubicTo(l-3*scale,b-27*scale,l-2*scale,t+25*scale,l+5*scale,t);
    face.closeSubPath();
    juce::DropShadow(juce::Colours::black.withAlpha(0.75f),juce::roundToInt(9*scale),{juce::roundToInt(8*scale),juce::roundToInt(9*scale)}).drawForPath(g,face);
    g.setColour(metal(0xff111415));g.fillPath(face,juce::AffineTransform::translation(6*scale,6*scale));
    juce::ColourGradient finish(metal(0xffb0aa99),cx,t,metal(0xffe2d9c5),cx,b,false);
    finish.addColour(0.025,metal(0xfffff4db));finish.addColour(0.07,metal(0xff746d5d));
    finish.addColour(0.23,metal(0xff494538));finish.addColour(0.47,metal(0xff898270));
    finish.addColour(0.66,metal(0xffd0c6b1));finish.addColour(0.86,metal(0xfffff4df));
    finish.addColour(0.96,metal(0xffdfd4bf));
    g.setGradientFill(finish);g.fillPath(face);
    {
        juce::Graphics::ScopedSaveState saved(g);g.reduceClipRegion(face);
        juce::ColourGradient side(juce::Colours::white.withAlpha(0.23f),l,t,juce::Colours::transparentWhite,r,t,false);
        side.addColour(0.1,juce::Colours::transparentWhite);side.addColour(0.86,juce::Colours::transparentWhite);side.addColour(1,juce::Colours::black.withAlpha(0.35f));
        g.setGradientFill(side);g.fillRect(l,t,w,h);
        for(int row=0;row<(int)h;++row)
        {
            g.setColour((row%3==0?juce::Colours::white:juce::Colours::black).withAlpha(0.012f));
            g.drawHorizontalLine((int)t+row,l,r);
        }
    }
    g.setColour(metal(0xff121617));g.strokePath(face,juce::PathStrokeType(1.8f*scale));
    juce::Path lip;lip.startNewSubPath(l+6*scale,t+3*scale);lip.lineTo(r-7*scale,t+3*scale);
    g.setColour(juce::Colours::white.withAlpha(0.75f));g.strokePath(lip,juce::PathStrokeType(1.4f*scale));
    g.setColour(juce::Colours::black.withAlpha(0.7f));g.fillRect(l+9*scale,pos-1.5f*scale,w-18*scale,2.2f*scale);
    g.setColour(juce::Colours::white.withAlpha(0.7f));g.fillRect(l+9*scale,pos+0.7f*scale,w-18*scale,1.2f*scale);
    if(slider.isMouseOverOrDragging())
    {
        g.setColour(juce::Colours::white.withAlpha(0.22f));g.strokePath(face,juce::PathStrokeType(1.2f*scale));
    }
}
void VocalsRackEditor::FaderLook::drawRotarySlider(juce::Graphics& g,int x,int y,int w,int h,float position,float,float,juce::Slider& slider)
{
    auto r=juce::Rectangle<float>((float)x,(float)y,(float)w,(float)h).reduced(4);
    g.setOpacity(slider.isEnabled()?1.0f:0.45f);
    g.setColour(juce::Colours::black);g.fillEllipse(r.translated(2,3));
    g.setGradientFill(juce::ColourGradient(metal(0xffc3c6c2),r.getX(),r.getY(),metal(0xff242727),r.getRight(),r.getBottom(),false));g.fillEllipse(r);
    auto cap=r.reduced(3);g.setGradientFill(juce::ColourGradient(metal(0xff863f41),cap.getX(),cap.getY(),metal(0xff2e0d12),cap.getRight(),cap.getBottom(),false));g.fillEllipse(cap);
    g.setColour(metal(0xffb38985));g.drawEllipse(cap,1);
    const float angle=-2.35f+position*4.7f;
    auto c=r.getCentre();auto tip=c+juce::Point<float>(std::sin(angle),-std::cos(angle))*cap.getWidth()*0.42f;
    g.setColour(metal(0xfff7eadd));g.drawLine(c.x,c.y,tip.x,tip.y,2);
}
void VocalsRackEditor::FaderLook::drawButtonBackground(juce::Graphics& g,juce::Button& button,const juce::Colour&,bool hover,bool down)
{
    auto r=button.getLocalBounds().toFloat();
    if(button.getProperties().getWithDefault("hardwareSwitch",false))
    {
        const float k=r.getHeight()/58.0f;
        juce::DropShadow(juce::Colours::black.withAlpha(0.7f),juce::roundToInt(5*k),{juce::roundToInt(4*k),juce::roundToInt(5*k)}).drawForRectangle(g,r.reduced(4*k).toNearestInt());
        bezel(g,r.reduced(3*k));
        auto lamp=r.reduced(10*k,9*k).translated(0,down?1.5f*k:0);
        const int mode=(int)button.getProperties().getWithDefault("colourMode",0);
        const bool lit=mode>0;
        juce::ColourGradient light(lit?metal(0xffffe9a4):metal(0xff68683e),lamp.getX(),lamp.getY(),lit?metal(0xffa17a2d):metal(0xff3a3e27),lamp.getRight(),lamp.getBottom(),false);
        light.addColour(0.45,lit?metal(0xffccb25b):metal(0xff50523a));g.setGradientFill(light);g.fillRect(lamp);
        g.setColour(juce::Colours::white.withAlpha(hover?0.65f:0.38f));g.drawLine(lamp.getX(),lamp.getY(),lamp.getRight(),lamp.getY(),k);
        g.setColour(juce::Colours::black.withAlpha(0.5f));g.drawRect(lamp,0.8f*k);
    }
    else
    {
        g.setGradientFill(juce::ColourGradient(metal(down?0xff121515:0xff383c3d),0,0,metal(0xff090b0b),0,r.getHeight(),false));g.fillRect(r);
        g.setColour(hover?metal(0xffbec2bd):metal(0xff6b706e));g.drawRect(r.reduced(0.5f),1);
        g.setColour(metal(0xff090a0a));g.drawLine(1,r.getBottom()-2,r.getRight()-1,r.getBottom()-2);
    }
}
void VocalsRackEditor::FaderLook::drawButtonText(juce::Graphics& g,juce::TextButton& b,bool,bool down)
{
    if(b.getProperties().getWithDefault("hardwareSwitch",false))
    {
        const float k=(float)b.getHeight()/58.0f,cx=(float)b.getWidth()*0.5f,cy=(float)b.getHeight()*0.5f+(down?1.5f*k:0);
        juce::Path arrow;arrow.startNewSubPath(cx-6*k,cy);arrow.lineTo(cx,cy-6*k);arrow.lineTo(cx+6*k,cy);arrow.lineTo(cx+2*k,cy);arrow.lineTo(cx+2*k,cy+7*k);arrow.lineTo(cx-2*k,cy+7*k);arrow.lineTo(cx-2*k,cy);arrow.closeSubPath();
        g.setColour(juce::Colours::white.withAlpha(0.6f));g.fillPath(arrow,juce::AffineTransform::translation(0,k));
        g.setColour(metal(0xff34371b));g.fillPath(arrow);
    }
    else
    {
        g.setColour(metal(0xffc0c3be));g.setFont(juce::FontOptions((float)b.getHeight()*0.6f));g.drawText(b.getButtonText(),b.getLocalBounds().reduced(2),juce::Justification::centred);
    }
}
void VocalsRackEditor::FaderLook::drawToggleButton(juce::Graphics& g,juce::ToggleButton& b,bool hover,bool down)
{
    drawButtonBackground(g,b,juce::Colours::black,hover,down);
    g.setColour(b.getToggleState()?metal(0xffe3ce5c):metal(0xff9b9d97));g.setFont(juce::FontOptions((float)b.getHeight()*0.55f));
    g.drawText(b.getButtonText(),b.getLocalBounds(),juce::Justification::centred);
}
void VocalsRackEditor::FaderLook::drawComboBox(juce::Graphics& g,int w,int h,bool,int,int,int,int,juce::ComboBox& box)
{
    bezel(g,{1,1,(float)w-2,(float)h-2});
    g.setGradientFill(juce::ColourGradient(metal(0xff28261c),0,2,metal(0xff121510),0,(float)h-2,false));
    g.fillRect(4,4,w-8,h-8);
    const float cx=(float)w-12,cy=(float)h*0.5f;
    juce::Path arrow;arrow.addTriangle(cx-4,cy-2,cx+4,cy-2,cx,cy+3);
    g.setColour(metal(0xffc8aa6a));g.fillPath(arrow);
    if(box.getProperties().getWithDefault("modified",false))
    {g.setColour(metal(0xffd6a24d));g.fillEllipse((float)w-32,cy-2,4,4);}
}
juce::Font VocalsRackEditor::FaderLook::getComboBoxFont(juce::ComboBox& box)
{
    return juce::Font(juce::FontOptions((float)box.getHeight()*0.56f));
}
void VocalsRackEditor::FaderLook::positionComboBoxText(juce::ComboBox& box,juce::Label& label)
{
    label.setBounds(9,1,box.getWidth()-47,box.getHeight()-2);
    label.setFont(getComboBoxFont(box));
}
VocalsRackEditor::VocalsRackEditor(VocalsRackProcessor& p) : AudioProcessorEditor(p), processor(p)
{
    reference = juce::ImageCache::getFromMemory(BinaryData::vocalsrackreference_png, BinaryData::vocalsrackreference_pngSize);
    look.setColour(juce::PopupMenu::backgroundColourId,metal(0xff24231d));
    look.setColour(juce::PopupMenu::textColourId,metal(0xffe3d6b4));
    look.setColour(juce::PopupMenu::highlightedBackgroundColourId,metal(0xff645139));
    look.setColour(juce::PopupMenu::highlightedTextColourId,metal(0xffffe8b8));
    const char* names[] = {"Input Sensitivity", "Bass", "Treble", "Compress", "Reverb", "Delay", "Pitch", "Output", "Balance"};
    for (size_t i = 0; i < sliders.size(); ++i)
    {
        auto& s = sliders[i]; s.setName(names[i]); s.setTitle(names[i]);
        s.setSliderStyle(i == 8 ? juce::Slider::RotaryVerticalDrag : juce::Slider::LinearVertical);
        s.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0); s.setDoubleClickReturnValue(true, 0);
        s.setPopupDisplayEnabled(true, true, this); s.setTextValueSuffix(" dB");
        s.setLookAndFeel(&look);
        addAndMakeVisible(s);
        attachments[i] = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(p.state, p.controls[i], s);
    }
    for (size_t i = 0; i < modes.size(); ++i)
    {
        auto& b = modes[i]; b.setName(juce::String(names[i+1])+" Color"); b.setTitle(b.getName());
        b.getProperties().set("hardwareSwitch",true); b.setLookAndFeel(&look); addAndMakeVisible(b);
        b.onClick = [this, i] {
            auto* parameter = processor.state.getParameter(processor.colours[i]);
            const int next = ((int)processor.state.getRawParameterValue(processor.colours[i])->load() + 1) % 4;
            parameter->beginChangeGesture(); parameter->setValueNotifyingHost(parameter->convertTo0to1((float)next)); parameter->endChangeGesture();
            timerCallback();
        };
    }
    for (auto* b : {&resetButton, &copyButton, &swapButton, &saveButton, &loadButton, &previousPreset, &nextPreset}) { b->setLookAndFeel(&look); addAndMakeVisible(b); }
    bypassButton.setLookAndFeel(&look); addAndMakeVisible(bypassButton);
    bypassAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(p.state, "bypass", bypassButton);
    presetBox.setName("Factory Presets");presetBox.setTitle("60 presets por genero");
    presetBox.setLookAndFeel(&look);presetBox.setColour(juce::ComboBox::textColourId,metal(0xffd9d7a3));
    presetBox.setTextWhenNothingSelected("60 PRESETS / SELECIONE");
    for(int category=0;category<6;++category)
    {
        juce::PopupMenu group;
        for(int i=0;i<60;++i) if(RackFactory::presets[(size_t)i].category==category)
            group.addItem(i+1,VocalsRackProcessor::factoryPresetName(i));
        presetBox.getRootMenu()->addSubMenu(VocalsRackProcessor::factoryCategoryName(category),group);
    }
    addAndMakeVisible(presetBox);
    presetBox.onChange=[this] { if(presetBox.getSelectedId()>0) processor.loadFactoryPreset(presetBox.getSelectedId()-1);timerCallback(); };
    previousPreset.setName("Previous Preset");nextPreset.setName("Next Preset");
    previousPreset.onClick=[this] { int i=processor.getSelectedFactoryPreset();processor.loadFactoryPreset(i<0?59:(i+59)%60);timerCallback(); };
    nextPreset.onClick=[this] { processor.loadFactoryPreset((processor.getSelectedFactoryPreset()+1)%60);timerCallback(); };
    resetButton.onClick = [this] { processor.setCurrentProgram(0);timerCallback(); };
    copyButton.onClick = [this] { alternate = processor.capturePresetState(); };
    swapButton.onClick = [this] { if (!alternate.isValid()) alternate = processor.capturePresetState(); auto current = processor.capturePresetState(); processor.restorePresetState(alternate); alternate = current; isB = !isB; swapButton.setButtonText(isB ? "B / A" : "A / B"); };
    saveButton.onClick = [this] { chooseFile(true); }; loadButton.onClick = [this] { chooseFile(false); };
    setResizable(true, true); getConstrainer()->setFixedAspectRatio(baseW / baseH);
    setResizeLimits(778, 505, 1557, 1010); setSize(1090, 707);
    buildPanel(); timerCallback(); startTimerHz(30);
}
VocalsRackEditor::~VocalsRackEditor()
{
    stopTimer(); for(auto& s:sliders) s.setLookAndFeel(nullptr);
    for(auto& b:modes) b.setLookAndFeel(nullptr);
    for(auto* b:{&resetButton,&copyButton,&swapButton,&saveButton,&loadButton,&previousPreset,&nextPreset}) b->setLookAndFeel(nullptr);
    bypassButton.setLookAndFeel(nullptr);presetBox.setLookAndFeel(nullptr);
}
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
        modes[i].setButtonText(labels[i][m]);
        modes[i].setTooltip(modes[i].getName()+": "+labels[i][m]+" (click to cycle)");
        modes[i].getProperties().set("colourMode",m);
        modes[i].setColour(juce::TextButton::textColourOffId, modeColour(m));
        modes[i].setColour(juce::TextButton::buttonColourId, juce::Colour(0xff242729));
    }
    const int preset=processor.getSelectedFactoryPreset();
    presetBox.setSelectedId(preset+1,juce::dontSendNotification);
    const bool modified=processor.isFactoryPresetModified();
    presetBox.getProperties().set("modified",modified);
    presetBox.setTooltip(preset<0?"60 presets em seis categorias":VocalsRackProcessor::factoryPresetName(preset)+" - "+juce::String::fromUTF8(RackFactory::presets[(size_t)preset].description)+(modified?" (editado)":""));
    presetBox.repaint();
    sliders[8].setEnabled(processor.getTotalNumInputChannels() == 2);
    repaint();
}
void VocalsRackEditor::resized()
{
    const float sx = (float)getWidth() / baseW, sy = (float)getHeight() / baseH;
    auto bounds = [=](float x, float y, float w, float h) { return juce::Rectangle<int>((int)(x*sx), (int)(y*sy), (int)(w*sx), (int)(h*sy)); };
    for (size_t i = 0; i < 8; ++i) sliders[i].setBounds(bounds(centres[i] - 61, 447, 122, 440));
    sliders[8].setBounds(bounds(247, 840, 48, 48));
    for (size_t i = 0; i < 6; ++i) modes[i].setBounds(bounds(centres[i+1] - 38, 354, 76, 58));
    resetButton.setBounds(bounds(231,26,62,28));previousPreset.setBounds(bounds(300,26,32,28));
    presetBox.setBounds(bounds(339,25,658,31));nextPreset.setBounds(bounds(1004,26,32,28));
    copyButton.setBounds(bounds(1043,26,70,28));swapButton.setBounds(bounds(1120,26,70,28));
    loadButton.setBounds(bounds(1197,26,86,28));saveButton.setBounds(bounds(1290,26,91,28));
    bypassButton.setBounds(bounds(1388,26,130,28));
}
void VocalsRackEditor::buildPanel()
{
    // Construct a separate display layer. The immutable reference is never edited.
    panel=juce::Image(juce::Image::RGB,(int)baseW,(int)baseH,true,juce::SoftwareImageType());
    juce::Graphics g(panel);g.drawImage(reference,panel.getBounds().toFloat());
    g.setColour(metal(0xff8b673c).withAlpha(0.09f));g.fillAll();
    juce::Image grain(juce::Image::ARGB,192,192,true,juce::SoftwareImageType());
    juce::Random random(49176);
    for(int y=0;y<grain.getHeight();++y) for(int x=0;x<grain.getWidth();++x)
        grain.setPixelAt(x,y,(random.nextBool()?juce::Colours::white:juce::Colours::black).withAlpha((float)random.nextInt(10)/255.0f));
    auto surface=[&](juce::Rectangle<float> r)
    {
        juce::Graphics::ScopedSaveState save(g);g.reduceClipRegion(r.toNearestInt());
        juce::ColourGradient body(metal(0xff494135),r.getX(),r.getY(),metal(0xff29261f),r.getRight(),r.getBottom(),false);
        body.addColour(0.35,metal(0xff3e382e));body.addColour(0.7,metal(0xff383329));g.setGradientFill(body);g.fillRect(r);
        g.setTiledImageFill(grain,0,0,1);g.fillRect(r);
        for(int y=(int)r.getY();y<(int)r.getBottom();y+=3)
        {g.setColour(juce::Colours::white.withAlpha(0.012f));g.drawHorizontalLine(y,r.getX(),r.getRight());}
        juce::ColourGradient lighting(juce::Colours::white.withAlpha(0.035f),r.getX(),r.getY(),juce::Colours::black.withAlpha(0.15f),r.getRight(),r.getY(),false);
        lighting.addColour(0.5,juce::Colours::transparentBlack);g.setGradientFill(lighting);g.fillRect(r);
        // Fixed-seed patina: oxidised patches, small pits and worn brushed edges.
        for(int n=0;n<45;++n)
        {
            const float x=r.getX()+random.nextFloat()*r.getWidth(),y=r.getY()+random.nextFloat()*r.getHeight();
            g.setColour(metal(n%2?0xffa07c42:0xff151913).withAlpha(0.028f));
            g.fillEllipse(x,y,12+random.nextFloat()*60,6+random.nextFloat()*23);
        }
        for(int n=0;n<330;++n)
        {
            const float x=r.getX()+random.nextFloat()*r.getWidth(),y=r.getY()+random.nextFloat()*r.getHeight();
            const float length=1+random.nextFloat()*10;
            g.setColour(metal(n%3?0xff211c14:0xffb39a63).withAlpha(0.2f));g.drawLine(x,y,x+length,y,0.6f);
        }
        g.setColour(metal(0xffcdb586).withAlpha(0.18f));g.drawLine(r.getX(),r.getY()+1,r.getRight(),r.getY()+1,1);
        g.setColour(juce::Colours::black.withAlpha(0.7f));g.drawLine(r.getX(),r.getBottom()-1,r.getRight(),r.getBottom()-1,2);
    };
    surface({57,392,294,549});surface({355,240,850,701});surface({1209,392,290,549});
    for(float x:{351.0f,1205.0f})
    {
        g.setColour(juce::Colours::black);g.fillRect(x,240.0f,4.0f,709.0f);
        g.setColour(juce::Colours::white.withAlpha(0.13f));g.fillRect(x+4,240.0f,1.0f,700.0f);
    }
    // Recessed display with a steel rim and a fine mesh behind the legends.
    bezel(g,{372,253,819,91});
    g.setColour(metal(0xff202728));g.fillRect(380,261,803,75);
    for(int y=263;y<336;y+=4) for(int x=382;x<1183;x+=4)
    {g.setColour(juce::Colours::black.withAlpha(0.35f));g.fillRect(x,y,1,1);}
    const char* channelNames[]={"INPUT SENS","BASS","TREBLE","COMPRESS","REVERB","DELAY","PITCH","OUTPUT"};
    for(size_t i=0;i<8;++i)
    {
        const float cx=centres[i];
        if(i!=0) screw(g,cx,451);
        screw(g,cx,879);
        // Bevel, recessed black slot, and a narrow reflected rim.
        g.setColour(metal(0xff656a66));g.fillRoundedRectangle(cx-6,474,12,386,6);
        g.setColour(metal(0xff111516));g.fillRoundedRectangle(cx-4.5f,476,9,382,4);
        g.setColour(juce::Colours::black);g.fillRoundedRectangle(cx-3,477,6,380,3);
        g.setColour(juce::Colours::white.withAlpha(0.13f));g.drawLine(cx+5.5f,481,cx+5.5f,852,1);
        for(int tick=0;tick<3;++tick)
        {
            const float y=513.0f+(float)tick*154.0f;
            const bool right=i==7;
            const float tx=right?cx+52:cx-94;
            g.setColour(metal(0xffbaad8c));g.setFont(juce::FontOptions(17.0f));
            g.drawText(tick==0?"+10":tick==1?"0":"-10",(int)tx,(int)y-12,43,24,juce::Justification::centred);
            g.drawLine(right?tx-12:tx+45,y,right?tx-3:tx+54,y,1.3f);
        }
        g.setColour(juce::Colours::black.withAlpha(0.65f));g.setFont(juce::FontOptions(18.5f,juce::Font::bold));
        g.drawText(channelNames[i],(int)cx-81,914,164,28,juce::Justification::centred);
        g.setColour(metal(0xffd7c7a3));g.drawText(channelNames[i],(int)cx-82,913,164,28,juce::Justification::centred);
    }
    // Meter cages, mounting screws and scale markings are static hardware.
    for(float x:{232.0f,1250.0f})
    {
        auto r=juce::Rectangle<float>(x,466,83,371);bezel(g,r);
        screw(g,x+41,436);
        if(x>1000) screw(g,x+41,868);
    }
    g.setColour(metal(0xffbcad8a));g.setFont(juce::FontOptions(14.0f,juce::Font::italic));
    g.drawText("L",230,853,17,25,juce::Justification::centred);g.drawText("R",298,853,17,25,juce::Justification::centred);
    // Lower rail highlight ties the three sections into the same metal chassis.
    juce::ColourGradient rail(metal(0xff5a4c37),0,941,metal(0xff181813),0,963,false);rail.addColour(0.3,metal(0xff302a20));
    g.setGradientFill(rail);g.fillRect(57,941,1442,22);
    g.setColour(juce::Colours::white.withAlpha(0.1f));g.drawLine(58,942,1498,942,1);
    // Oiled walnut side cheeks, machined brass edging and fine vertical grain.
    for(float x:{32.0f,1514.0f})
    {
        juce::Rectangle<float> cheek(x,177,16,738);
        g.setGradientFill(juce::ColourGradient(metal(0xff251a10),x,0,metal(0xff6b4729),x+16,0,false));g.fillRoundedRectangle(cheek,5);
        for(int n=0;n<13;++n)
        {
            juce::Path fibre;const float fx=x+1+(float)n;
            fibre.startNewSubPath(fx,181);fibre.cubicTo(fx-2,390,fx+2,690,fx,910);
            g.setColour(metal(n%2?0xffb0814a:0xff1c140c).withAlpha(0.23f));g.strokePath(fibre,juce::PathStrokeType(0.6f));
        }
        g.setColour(metal(0xffb18c51).withAlpha(0.5f));g.drawRoundedRectangle(cheek,5,0.8f);
    }
    g.setFont(juce::FontOptions(13.0f,juce::Font::bold));g.setColour(metal(0xffc9b58d).withAlpha(0.75f));
    g.drawText("VINTAGE  /  VOCAL PROCESSOR",790,209,400,21,juce::Justification::centredRight);
    // Clear the old toolbar only; keep the reference's red top and outer enclosure.
    g.setColour(metal(0xff14140f));g.fillRect(21,24,1515,33);
    g.setColour(metal(0xff897752));g.drawRect(22,25,1512,31,1);
    g.setFont(juce::FontOptions(19.0f,juce::Font::bold));g.setColour(metal(0xffdcc8a0));g.drawText("VOCALS RACK",32,26,191,28,juce::Justification::centredLeft);

}
void VocalsRackEditor::paint(juce::Graphics& g)
{
    g.drawImage(panel,getLocalBounds().toFloat());
    g.addTransform(juce::AffineTransform::scale((float)getWidth()/baseW,(float)getHeight()/baseH));

    for(int i=0;i<6;++i)
    {
        const int selected=(int)processor.state.getRawParameterValue(processor.colours[i])->load();
        for(int m=1;m<=3;++m)
        {
            auto r=juce::Rectangle<int>((int)centres[i+1]-62,257+(3-m)*25,124,25);
            g.setFont(juce::FontOptions(17.0f,juce::Font::bold|juce::Font::italic));
            if(selected==m)
            {
                g.setColour(modeColour(m).withAlpha(0.13f));g.drawText(labels[i][m].toUpperCase(),r.expanded(1),juce::Justification::centred);
            }
            g.setColour(selected==m?modeColour(m):metal(0xff6a6350));g.drawText(labels[i][m].toUpperCase(),r,juce::Justification::centred);
        }
    }
    for(int side=0;side<2;++side)
    {
        const float x=side==0?240.0f:1258.0f;
        for(int ch=0;ch<2;++ch)
        {
            const auto index=(size_t)(side*2+ch);
            const float db=juce::Decibels::gainToDecibels(processor.peaks[index].load(),-100.0f);
            const float amount=juce::jlimit(0.0f,1.0f,(db+26)/26);
            const float bx=x+(float)ch*47;
            for(int seg=0;seg<39;++seg)
            {
                const float y=821.0f-(float)seg*8.4f;const bool lit=(float)seg/39<amount;
                const auto colour=lit?metal(0xffb9df61):metal(0xff2b3520);
                if(lit)
                {g.setColour(colour.withAlpha(0.12f));g.fillRect(bx-1,y-1,21.0f,8.0f);}
                juce::ColourGradient led(colour.brighter(lit?0.15f:0.0f),bx,y,colour.darker(0.3f),bx,y+6,false);
                g.setGradientFill(led);g.fillRect(bx,y,19.0f,6.2f);
            }
            g.setColour(processor.clips[index].load()?metal(0xffff4f35):metal(0xff61282b));g.fillRect(bx,484.0f,19.0f,8.0f);
        }
        g.setFont(juce::FontOptions(14.0f,juce::Font::bold|juce::Font::italic));g.setColour(metal(0xffc4b78f));
        for(int tick=0;tick<5;++tick)g.drawText(juce::String(tick*6),(int)x+21,481+tick*76,24,21,juce::Justification::centred);
        // A narrow reflection on the meter glass rather than an opaque overlay.
        g.setGradientFill(juce::ColourGradient(juce::Colours::white.withAlpha(0.06f),x,477,juce::Colours::transparentWhite,x+25,477,false));g.fillRect(x-1,477.0f,26.0f,350.0f);
    }
    const float inputDb=juce::Decibels::gainToDecibels(std::max(processor.peaks[0].load(),processor.peaks[1].load()),-100.0f);
    const auto lamp=inputDb>-1?metal(0xfff53e24):inputDb>-12?metal(0xffe9dd51):inputDb>-26?metal(0xff83db44):metal(0xff464832);
    bezel(g,{130,422,40,29});
    g.setGradientFill(juce::ColourGradient(lamp.brighter(0.4f),137,428,lamp.darker(0.3f),164,444,false));g.fillRoundedRectangle(137,428,26,16,4);
    g.setColour(juce::Colours::white.withAlpha(0.3f));g.drawLine(141,430,158,430,1);
}
void VocalsRackEditor::mouseDown(const juce::MouseEvent& e)
{
    const float x = (float)e.x * baseW/(float)getWidth(), y = (float)e.y*baseH/(float)getHeight();
    if (y >= 470 && y <= 835)
        for (int side = 0; side < 2; ++side)
            if (x >= (side == 0 ? 230 : 1240) && x <= (side == 0 ? 315 : 1335))
                for (int ch = 0; ch < 2; ++ch) processor.clips[(size_t)(side*2+ch)].store(false);
}
