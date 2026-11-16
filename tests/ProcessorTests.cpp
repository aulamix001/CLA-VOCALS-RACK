#include "../Source/PluginProcessor.h"
#include <cmath>
#include <iostream>
#include <stdexcept>
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
int main(int argc, char** argv)
{
    try {
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
            juce::Image image(juce::Image::ARGB, editor->getWidth(), editor->getHeight(), true);
            juce::Graphics graphics(image); editor->paintEntireComponent(graphics, true);
            juce::FileOutputStream stream { juce::File(argv[2]) };
            require(stream.openedOk() && juce::PNGImageFormat().writeImageToStream(image, stream), "render PNG");
            std::cout << "PASS editor construction, fader attachment and offscreen rendering\n";
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
