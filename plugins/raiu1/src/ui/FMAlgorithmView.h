#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <array>

namespace raiu::ui {

// FMアルゴリズムの変調ルーティングをノード図として可視化する。
// ノードの明るさはそのオペレーターのLevelを反映し、矢印は変調の流れを示す。
// クリックで次のアルゴリズムへ、右クリックで前のアルゴリズムへ切り替わる。
class FMAlgorithmView : public juce::Component, private juce::Timer {
public:
    explicit FMAlgorithmView(juce::AudioProcessorValueTreeState& state);
    ~FMAlgorithmView() override;

    void paint(juce::Graphics&) override;
    void mouseUp(const juce::MouseEvent&) override;

private:
    void timerCallback() override;

    struct Layout {
        std::array<juce::Point<float>, 4> pos;   // 正規化座標 0..1
        std::array<std::pair<int, int>, 4> arrows;  // {from, to}、未使用は{-1,-1}
        int numArrows;
        uint8_t carrierMask;
        const char* description;
    };
    static const std::array<Layout, 8>& layouts();

    juce::AudioProcessorValueTreeState& apvts;
    std::array<float, 4> smoothedLevel{};
    float smoothedFeedback = 0.0f;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(FMAlgorithmView)
};

} // namespace raiu::ui
