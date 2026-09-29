#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <vector>

namespace raiu::ui {

// NATURE OSC のモード選択を「アイコン付きボタン」+「有機的アニメーション」で見せる。
// 選んだ自然音の質感が視覚的に生きて動くことで、パラメーターの意味を直感的に伝える。
class NatureOscPad : public juce::Component, private juce::Timer {
public:
    explicit NatureOscPad(juce::AudioProcessorValueTreeState& state);
    ~NatureOscPad() override;

    void paint(juce::Graphics&) override;
    void resized() override;
    void mouseUp(const juce::MouseEvent&) override;

private:
    void timerCallback() override;
    void spawnAndUpdateParticles();
    juce::Rectangle<int> buttonBounds(int index) const;

    struct Particle {
        float x, y, vx, vy, life, maxLife, phase;
    };

    juce::AudioProcessorValueTreeState& apvts;
    std::vector<Particle> particles;
    juce::Random rng;
    int lastMode = -1;
    double t = 0.0;

    juce::Rectangle<int> visualArea;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(NatureOscPad)
};

} // namespace raiu::ui
