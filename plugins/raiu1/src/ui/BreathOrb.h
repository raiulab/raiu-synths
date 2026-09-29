#pragma once

#include <juce_audio_processors/juce_audio_processors.h>

namespace raiu::ui {

// Breath / Velocity->Texture / Swell を3層の同心円として可視化する
// 「呼吸するコア」。FMとテクスチャが相互作用する様子を有機的に表現する。
class BreathOrb : public juce::Component, private juce::Timer {
public:
    explicit BreathOrb(juce::AudioProcessorValueTreeState& state);
    ~BreathOrb() override;

    void paint(juce::Graphics&) override;

private:
    void timerCallback() override;

    juce::AudioProcessorValueTreeState& apvts;
    float breathS = 0, velS = 0, swellS = 0;
    double t = 0.0;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(BreathOrb)
};

} // namespace raiu::ui
