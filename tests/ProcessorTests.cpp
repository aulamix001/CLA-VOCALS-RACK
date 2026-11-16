#include "../Source/PluginProcessor.h"
#include <cmath>
#include <iostream>
#include <stdexcept>
#include <set>
static void require(bool ok, const char* message) { if (!ok) throw std::runtime_error(message); }
static void set(VocalsRackProcessor& p, const char* id, float value)
{
    auto* parameter = p.state.getParameter(id);
    parameter->setValueNotifyingHost(parameter->convertTo0to1(value));
}
static void clearModes(VocalsRackProcessor& p) { for (auto* id : p.colours) set(p, id, 0); }
static void process(VocalsRackProcessor& p, juce::AudioBuffer<float>& b) { juce::MidiBuffer midi; p.processBlock(b, midi); }
class Tempo final : public juce::AudioPlayHead
{
public:
    juce::Optional<PositionInfo> getPosition() const override { PositionInfo p; p.setBpm(120); return p; }
};
static void testFactoryBank()
{
    VocalsRackProcessor p, recall;
    require(p.getNumPrograms()==61,"host exposes initial state plus 60 factory programs");
    std::array<int,6> categoryCounts {};
    std::set<std::string> names;
    std::set<std::array<float,15>> configurations;
    p.prepareToPlay(48000,256);
    for(int index=0;index<60;++index)
    {
        const auto& preset=RackFactory::presets[(size_t)index];
        require(preset.category>=0 && preset.category<6,"valid factory category");
        ++categoryCounts[(size_t)preset.category];
        require(names.insert(VocalsRackProcessor::factoryPresetName(index).toStdString()).second,"unique preset name");
        std::array<float,15> fingerprint {};
        require(p.loadFactoryPreset(index),"load factory preset");
        require(p.getCurrentProgram()==index+1 && !p.isFactoryPresetModified(),"factory program identity and clean state");
        for(size_t i=0;i<9;++i)
        {
            const float value=p.state.getRawParameterValue(p.controls[i])->load();
            require(std::abs(value-preset.faders[i])<0.002f,"preset maps every fader to its declared parameter");
            require(value>=-10 && value<=10 && (i!=8 || (value>=-6 && value<=6)),"preset fader within supported range");
            fingerprint[i]=preset.faders[i];
        }
        for(size_t i=0;i<6;++i)
        {
            require(preset.colours[i]>=0 && preset.colours[i]<=3,"valid preset colour");
            require((int)p.state.getRawParameterValue(p.colours[i])->load()==preset.colours[i],"preset maps all effect modes");
            fingerprint[i+9]=(float)preset.colours[i];
        }
        require(configurations.insert(fingerprint).second,"60 distinct configurations, not renamed duplicates");
        require(p.state.getRawParameterValue("bypass")->load()<0.5f,"factory preset enables processing");
        juce::MemoryBlock saved;p.getStateInformation(saved);recall.setStateInformation(saved.getData(),(int)saved.getSize());
        require(recall.getSelectedFactoryPreset()==index && !recall.isFactoryPresetModified(),"recall factory identity and complete state");
        double energy=0;
        for(int block=0;block<30;++block)
        {
            juce::AudioBuffer<float> audio(2,256);
            for(int n=0;n<256;++n)
            {
                const float t=(float)(block*256+n)/48000.0f;
                const float sample=0.12f*std::sin(juce::MathConstants<float>::twoPi*220*t)+0.03f*std::sin(juce::MathConstants<float>::twoPi*1760*t);
                audio.setSample(0,n,sample);audio.setSample(1,n,sample*0.9f);
            }
            process(p,audio);
            for(int ch=0;ch<2;++ch)for(int n=0;n<256;++n)
            {const float sample=audio.getSample(ch,n);require(std::isfinite(sample),"every factory preset produces finite audio");energy+=(double)sample*sample;}
        }
        require(energy>0.01,"every factory preset produces non-silent audio");
        set(p,"bass",preset.faders[1]+0.1f);require(p.isFactoryPresetModified(),"manual edit marks preset as modified");
        p.getStateInformation(saved);recall.setStateInformation(saved.getData(),(int)saved.getSize());
        require(recall.getSelectedFactoryPreset()==index && recall.isFactoryPresetModified(),"edited factory preset survives session recall");
        p.setCurrentProgram(index+1);require(!p.isFactoryPresetModified(),"host program selection reloads factory values");
    }
    for(int count:categoryCounts)require(count==10,"exactly ten presets in every requested genre");
    const int before=p.getCurrentProgram();
    require(!p.loadFactoryPreset(-1) && !p.loadFactoryPreset(60),"invalid factory IDs rejected");
    p.setCurrentProgram(-1);p.setCurrentProgram(61);require(p.getCurrentProgram()==before,"invalid host program preserves state");
    p.loadFactoryPreset(0);auto a=p.capturePresetState();p.loadFactoryPreset(59);p.restorePresetState(a);
    require(p.getSelectedFactoryPreset()==0 && !p.isFactoryPresetModified(),"A/B snapshot restores name and faders");
    auto legacy=p.capturePresetState();legacy.removeProperty("factoryPreset",nullptr);p.restorePresetState(legacy);
    require(p.getSelectedFactoryPreset()==-1,"legacy sessions without factory metadata remain supported");
    p.setCurrentProgram(0);require(p.getCurrentProgram()==0,"initial program reset");
    for(auto* id:p.controls)require(std::abs(p.state.getRawParameterValue(id)->load())<0.001f,"initial faders restored");
    std::cout<<"PASS 60 unique factory presets, six genres, complete mapping, host programs, state recall, edits, A/B and finite audio\n";
}

int main(int argc, char** argv)
{
    try {
        if(argc==2 && juce::String(argv[1])=="--presets") { testFactoryBank();return 0; }
        if (argc == 3 && juce::String(argv[1]) == "--render")
        {
            juce::ScopedJuceInitialiser_GUI gui;
            VocalsRackProcessor p; p.prepareToPlay(48000, 128);
            std::unique_ptr<juce::AudioProcessorEditor> editor(p.createEditor());
            require(editor != nullptr, "editor created");
            bool found = false;
            for (auto* child : editor->getChildren())
                if (auto* slider = dynamic_cast<juce::Slider*>(child))
                    if (slider->getName() == "Bass") { slider->setValue(4.2, juce::sendNotificationSync); found = true; }
            require(found && std::abs(p.state.getRawParameterValue("bass")->load() - 4.2f) < 0.001f, "editor fader changes audio parameter");
            // Exercise live controls, their full travel and the four mode states.
            const float fixture[] = {2.0f,-3.0f,4.0f,0.0f,2.0f,-4.0f,6.0f,0.0f,0.0f};
            const int fixtureModes[] = {1,2,3,1,2,2};
            int sliderCount=0,modeCount=0;
            for(auto* child:editor->getChildren())
            {
                if(auto* slider=dynamic_cast<juce::Slider*>(child))
                {
                    const int index=sliderCount++;
                    for(double endpoint:{slider->getMinimum(),slider->getMaximum()})
                    {
                        slider->setValue(endpoint,juce::sendNotificationSync);
                        require(std::abs(p.state.getRawParameterValue(p.controls[index])->load()-(float)endpoint)<0.001f,"full fader travel updates parameter");
                    }
                    slider->setValue(fixture[index],juce::sendNotificationSync);
                }
                if(auto* button=dynamic_cast<juce::TextButton*>(child))
                    if(button->getName().endsWith(" Color"))
                    {
                        const auto* id=p.colours[modeCount++];
                        for(int click=0;click<4;++click)
                        {
                            button->onClick();
                            require((int)p.state.getRawParameterValue(id)->load()==(click+2)%4,"mode button cycles through clear and three colours");
                        }
                        for(int click=1;click<fixtureModes[modeCount-1];++click) button->onClick();
                    }
            }
            require(sliderCount==9 && modeCount==6,"all hardware controls exercised");
            for(auto size:{juce::Point<int>(778,505),juce::Point<int>(1557,1010),juce::Point<int>(1090,707)})
            {
                editor->setSize(size.x,size.y);
                for(auto* child:editor->getChildren())require(editor->getLocalBounds().contains(child->getBounds()),"resized controls remain in panel");
            }
            juce::ComboBox* factoryBox=nullptr;
            for(auto* child:editor->getChildren()) if(auto* box=dynamic_cast<juce::ComboBox*>(child)) if(box->getName()=="Factory Presets") factoryBox=box;
            require(factoryBox!=nullptr,"factory selector present");
            for(int index=0;index<60;++index)
            {
                factoryBox->setSelectedId(index+1,juce::sendNotificationSync);
                require(p.getSelectedFactoryPreset()==index && !p.isFactoryPresetModified(),"menu loads selected factory configuration");
                int i=0;
                for(auto* child:editor->getChildren())if(auto* slider=dynamic_cast<juce::Slider*>(child))
                    require(std::abs((float)slider->getValue()-RackFactory::presets[(size_t)index].faders[(size_t)i++])<0.002f,"all visible faders follow preset loading");
            }
            factoryBox->setSelectedId(52,juce::sendNotificationSync); // Warm Fingerstyle demonstration.
            p.peaks[0].store(0.22f);p.peaks[1].store(0.18f);p.peaks[2].store(0.7f);p.peaks[3].store(0.78f);
            juce::Image image(juce::Image::ARGB, editor->getWidth(), editor->getHeight(), true, juce::SoftwareImageType());
            {
                juce::Graphics graphics(image);
                editor->paintEntireComponent(graphics, true);
            } // Finish the drawing context before inspecting or encoding pixels.
            int visiblePixels=0;
            for(int y=0;y<image.getHeight();y+=17) for(int x=0;x<image.getWidth();x+=17)
                if(image.getPixelAt(x,y).getBrightness()>0.12f) ++visiblePixels;
            require(visiblePixels>100,"editor screenshot contains rendered controls, not an empty surface");
            juce::FileOutputStream stream { juce::File(argv[2]) };
            require(stream.openedOk() && stream.setPosition(0) && stream.truncate().wasOk(), "replace previous editor render");
            require(stream.openedOk() && juce::PNGImageFormat().writeImageToStream(image, stream), "render PNG");
            std::cout << "PASS editor rendering, nine faders, six four-state switches and three window sizes\n";
            return 0;
        }
        VocalsRackProcessor p;
        for (auto* id : p.controls) require(std::abs(p.state.getRawParameterValue(id)->load()) < 0.001f, "default fader");
        for (auto* id : p.colours) require((int)p.state.getRawParameterValue(id)->load() == 1, "default mode");
        set(p, "bass", 7.3f); set(p, "delayColour", 3);
        juce::MemoryBlock state; p.getStateInformation(state);
        VocalsRackProcessor restored; restored.setStateInformation(state.getData(), (int)state.getSize());
        require(std::abs(restored.state.getRawParameterValue("bass")->load() - 7.3f) < 0.001f, "state float roundtrip");
        require((int)restored.state.getRawParameterValue("delayColour")->load() == 3, "state mode roundtrip");
        const char corrupt[] = "invalid"; restored.setStateInformation(corrupt, 7);
        require((int)restored.state.getRawParameterValue("delayColour")->load() == 3, "invalid state ignored");
        std::cout << "PASS parameters, defaults and state serialization\n";
        for (double sr : {44100.0, 48000.0, 96000.0})
        {
            VocalsRackProcessor dry; clearModes(dry); dry.prepareToPlay(sr, 128);
            juce::AudioBuffer<float> b(2, 128);
            for (int n = 0; n < 128; ++n) { b.setSample(0,n,0.2f); b.setSample(1,n,-0.1f); }
            process(dry,b);
            require(std::abs(b.getSample(0,100)-0.2f)<0.00001f, "clear modes dry left");
            require(std::abs(b.getSample(1,100)+0.1f)<0.00001f, "clear modes dry right");
            set(dry,"bypass",1); set(dry,"output",10); process(dry,b);
            require(std::abs(b.getSample(0,100)-0.2f)<0.00001f, "bypass unity");
            VocalsRackProcessor fx; fx.prepareToPlay(sr,128);
            for (int block = 0; block < 100; ++block)
            {
                for(int n=0;n<128;++n) { float x = 0.3f * std::sin((float)(block*128+n)*0.13f); b.setSample(0,n,x); b.setSample(1,n,x); }
                process(fx,b);
                for (int ch=0;ch<2;++ch) for(int n=0;n<128;++n) require(std::isfinite(b.getSample(ch,n)), "finite DSP output");
            }
        }
        std::cout << "PASS audio at 44.1/48/96 kHz, bypass and finite default processing\n";
        for (int module=0; module<6; ++module)
        {
            for (int mode=1;mode<=3;++mode)
            {
                VocalsRackProcessor fx; clearModes(fx); set(fx,fx.colours[module],(float)mode); set(fx,fx.controls[module+1],8);
                fx.prepareToPlay(48000,128);
                double difference=0;
                for(int block=0;block<240;++block)
                {
                    juce::AudioBuffer<float> b(2,128);
                    for(int n=0;n<128;++n) { float x=0.7f*std::sin((float)(block*128+n)*0.017f); b.setSample(0,n,x); b.setSample(1,n,x); }
                    auto dry=juce::AudioBuffer<float>(b); process(fx,b);
                    for(int n=0;n<128;++n) { require(std::isfinite(b.getSample(0,n)),"finite module"); difference+=std::abs(b.getSample(0,n)-dry.getSample(0,n)); }
                }
                require(difference>0.5,"module changes audio");
            }
        }
        std::cout << "PASS all 18 active module modes change audio\n";
        Tempo tempo;
        for (int mode=1;mode<=3;++mode)
        {
            VocalsRackProcessor d; clearModes(d); set(d,"delayColour",(float)mode); d.setPlayHead(&tempo); d.prepareToPlay(48000,64);
            const int expected = mode==1?6000:mode==2?12000:24000;
            int first=-1;
            for(int at=0;at<expected+64;at+=64)
            {
                juce::AudioBuffer<float> b(2,64); b.clear(); if(at==0) b.setSample(0,0,1);
                process(d,b);
                for(int n=0;n<64;++n) if(at+n>0 && std::abs(b.getSample(0,n))>0.0001f && first<0) first=at+n;
            }
            require(first==expected,"delay follows host BPM and note division");
            d.setPlayHead(nullptr);
        }
        std::cout << "PASS host-synchronized 1/16, 1/8, 1/4 delays\n";
        VocalsRackProcessor mono; auto layout=mono.getBusesLayout(); layout.inputBuses.set(0,juce::AudioChannelSet::mono());
        require(mono.setBusesLayout(layout),"mono to stereo supported"); clearModes(mono); mono.prepareToPlay(48000,128);
        juce::AudioBuffer<float> m(2,128); m.clear(); m.setSample(0,0,0.5f); process(mono,m);
        require(std::abs(m.getSample(1,0)-0.5f)<0.00001f,"mono duplicated to stereo");
        std::cout << "PASS mono-to-stereo routing\n";
        VocalsRackProcessor meter; clearModes(meter); meter.prepareToPlay(48000,128);
        m.clear(); m.setSample(0,0,1.5f); process(meter,m);
        require(meter.clips[0].load() && meter.clips[2].load(),"input/output clip latch");
        m.clear(); process(meter,m); require(meter.clips[0].load(),"clip remains latched");
        meter.reset(); require(!meter.clips[0].load(),"reset clears clipping");
        std::cout << "PASS peak metering and clipping\n";
    } catch(const std::exception& e) { std::cerr << "FAIL " << e.what() << '\n'; return 1; }
    return 0;
}
