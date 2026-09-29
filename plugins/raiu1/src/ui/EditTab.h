#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include "Knob.h"
#include "SectionPanel.h"

namespace raiu::ui {

// 「EDIT」タブ: FMオペレーター4基の全パラメーターを詳細に編集する上級者向け画面。
class EditTab : public juce::Component {
public:
    explicit EditTab(juce::AudioProcessorValueTreeState& apvts);
    void paint(juce::Graphics&) override;
    void resized() override;

private:
    SectionPanel opPanels[4]{
        {"Operator 1", Palette::fm}, {"Operator 2", Palette::fm},
        {"Operator 3", Palette::fm}, {"Operator 4", Palette::fm}};
    std::vector<std::unique_ptr<Knob>> opKnobs[4];
    juce::ToggleButton fixedButtons[4];
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> fixedAtts[4];

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(EditTab)
};

} // namespace raiu::ui
