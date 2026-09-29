#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include "Palette.h"

namespace raiu::ui {

// ラベル+ロータリースライダー+アタッチメントを1つにまとめた自己完結コンポーネント。
// アクセントカラーでどのセクションのパラメーターかを直感的に示す。
class Knob : public juce::Component {
public:
    Knob(juce::AudioProcessorValueTreeState& apvts, const juce::String& paramID,
        const juce::String& displayName, juce::Colour accent) {
        slider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
        slider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 56, 15);
        slider.setNumDecimalPlacesToDisplay(2);
        slider.setColour(juce::Slider::rotarySliderFillColourId, accent);
        addAndMakeVisible(slider);

        label.setText(displayName, juce::dontSendNotification);
        label.setJustificationType(juce::Justification::centred);
        label.setFont(juce::FontOptions(10.5f));
        label.setColour(juce::Label::textColourId, Palette::textMuted);
        addAndMakeVisible(label);

        att = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
            apvts, paramID, slider);
    }

    void resized() override {
        auto b = getLocalBounds();
        label.setBounds(b.removeFromTop(14));
        slider.setBounds(b);
    }

private:
    juce::Slider slider;
    juce::Label label;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> att;
};

// 横一列にノブを均等配置する
inline void layoutKnobRow(juce::Rectangle<int> area,
                          const std::vector<std::unique_ptr<Knob>>& knobs, int cellWidth) {
    for (auto& k : knobs) k->setBounds(area.removeFromLeft(cellWidth));
}

} // namespace raiu::ui
