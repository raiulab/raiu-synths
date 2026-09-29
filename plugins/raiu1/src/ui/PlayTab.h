#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include "Knob.h"
#include "SectionPanel.h"
#include "FMAlgorithmView.h"
#include "NatureOscPad.h"
#include "BreathOrb.h"

namespace raiu::ui {

// 「PLAY」タブ: 直感的なヒーロー画面。何を触ればどう変わるかが一目でわかる。
class PlayTab : public juce::Component {
public:
    explicit PlayTab(juce::AudioProcessorValueTreeState& apvts);

    void paint(juce::Graphics&) override;
    void resized() override;

private:
    std::vector<std::unique_ptr<Knob>> addRow(
        juce::AudioProcessorValueTreeState& apvts,
        const std::vector<std::pair<const char*, const char*>>& defs, juce::Colour accent);

    juce::AudioProcessorValueTreeState& state;

    SectionPanel fmPanel{"FM Engine", Palette::fm};
    SectionPanel naturePanel{"Nature Osc - playable", Palette::nature};
    SectionPanel breathPanel{"Interaction", Palette::interaction};
    SectionPanel texturePanel{"Ambient Texture", Palette::texture};
    SectionPanel globalPanel{"Global & FX", Palette::fx};

    std::unique_ptr<FMAlgorithmView> algoView;
    std::unique_ptr<NatureOscPad> naturePad;
    std::unique_ptr<BreathOrb> breathOrb;

    juce::ComboBox algoBox;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> algoAtt;
    juce::ToggleButton monoButton{"Mono"};
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> monoAtt;

    std::vector<std::unique_ptr<Knob>> fmKnobs, natureKnobs, breathKnobs, textureKnobs,
        globalKnobs, fxKnobs;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PlayTab)
};

} // namespace raiu::ui
