#include "PlayTab.h"

namespace raiu::ui {

std::vector<std::unique_ptr<Knob>> PlayTab::addRow(
    juce::AudioProcessorValueTreeState& apvts,
    const std::vector<std::pair<const char*, const char*>>& defs, juce::Colour accent) {
    std::vector<std::unique_ptr<Knob>> row;
    for (const auto& [id, name] : defs) {
        auto k = std::make_unique<Knob>(apvts, id, name, accent);
        addAndMakeVisible(*k);
        row.push_back(std::move(k));
    }
    return row;
}

PlayTab::PlayTab(juce::AudioProcessorValueTreeState& apvts) : state(apvts) {
    addAndMakeVisible(fmPanel);
    addAndMakeVisible(naturePanel);
    addAndMakeVisible(breathPanel);
    addAndMakeVisible(texturePanel);
    addAndMakeVisible(globalPanel);

    algoView = std::make_unique<FMAlgorithmView>(apvts);
    addAndMakeVisible(*algoView);
    naturePad = std::make_unique<NatureOscPad>(apvts);
    addAndMakeVisible(*naturePad);
    breathOrb = std::make_unique<BreathOrb>(apvts);
    addAndMakeVisible(*breathOrb);

    for (int a = 1; a <= 8; ++a) algoBox.addItem("Algorithm " + juce::String(a), a);
    addAndMakeVisible(algoBox);
    algoAtt = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(
        apvts, "algorithm", algoBox);
    addAndMakeVisible(monoButton);
    monoAtt = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(
        apvts, "mono", monoButton);

    fmKnobs = addRow(apvts,
                     {{"feedback", "Feedback"}, {"portamento", "Porta"},
                      {"vibrato_rate", "VibRate"}, {"vibrato_depth", "VibDep"},
                      {"master_gain", "Master"}},
                     Palette::fm);

    natureKnobs = addRow(apvts,
                         {{"nature_level", "Level"}, {"nature_keytrack", "KeyTrk"},
                          {"nature_pitch", "Pitch"}, {"nature_cutoff", "Cutoff"},
                          {"nature_res", "Res"}, {"nature_attack", "A"}, {"nature_decay", "D"},
                          {"nature_sustain", "S"}, {"nature_release", "R"}},
                         Palette::nature);

    breathKnobs = addRow(apvts,
                         {{"breath", "Breath"}, {"vel_to_texture", "Vel>Tex"},
                          {"swell", "Swell"}},
                         Palette::interaction);

    textureKnobs = addRow(apvts,
                          {{"tex_wind", "Wind"}, {"tex_water", "Water"}, {"tex_birds", "Birds"},
                           {"tex_insects", "Insects"}, {"tex_rain", "Rain"},
                           {"tex_rain_density", "RainDns"}, {"tex_resonance", "ResoQ"},
                           {"tex_resmix", "ResMix"}, {"tex_distance", "Dist"},
                           {"tex_master", "TexLvl"}},
                          Palette::texture);

    fxKnobs = addRow(apvts,
                     {{"chorus_rate", "ChRate"}, {"chorus_depth", "ChDep"},
                      {"chorus_mix", "ChMix"}, {"reverb_size", "RvSize"},
                      {"reverb_mix", "RvMix"}},
                     Palette::fx);
}

void PlayTab::resized() {
    auto full = getLocalBounds().reduced(10);
    auto topRow = full.removeFromTop(340);
    full.removeFromTop(8);
    auto texRow = full.removeFromTop(182);
    full.removeFromTop(8);
    auto fxRow = full.removeFromTop(118);

    // --- FM panel ---
    auto fm = topRow.removeFromLeft(300);
    topRow.removeFromLeft(8);
    fmPanel.setBounds(fm);
    fm.reduce(10, 10);
    fm.removeFromTop(20);
    algoView->setBounds(fm.removeFromTop(150));
    auto fmCtrl = fm.removeFromTop(28);
    algoBox.setBounds(fmCtrl.removeFromLeft(140));
    monoButton.setBounds(fmCtrl.withTrimmedLeft(8));
    fm.removeFromTop(6);
    layoutKnobRow(fm.removeFromTop(66), fmKnobs, fm.getWidth() / (int) fmKnobs.size());

    // --- Nature panel (hero) ---
    auto nature = topRow.removeFromLeft(400);
    topRow.removeFromLeft(8);
    naturePanel.setBounds(nature);
    nature.reduce(10, 10);
    nature.removeFromTop(20);
    naturePad->setBounds(nature.removeFromTop(150));
    nature.removeFromTop(4);
    int half = (int) natureKnobs.size() / 2 + 1;
    auto natRow1 = nature.removeFromTop(64);
    for (int i = 0; i < half; ++i)
        natureKnobs[(size_t) i]->setBounds(natRow1.removeFromLeft(natRow1.getWidth() / (half - i)));
    auto natRow2 = nature.removeFromTop(64);
    int remaining = (int) natureKnobs.size() - half;
    for (int i = 0; i < remaining; ++i)
        natureKnobs[(size_t) (half + i)]->setBounds(
            natRow2.removeFromLeft(natRow2.getWidth() / (remaining - i)));

    // --- Breath panel ---
    auto breath = topRow;
    breathPanel.setBounds(breath);
    breath.reduce(10, 10);
    breath.removeFromTop(20);
    breathOrb->setBounds(breath.removeFromTop(190));
    breath.removeFromTop(4);
    layoutKnobRow(breath.removeFromTop(66), breathKnobs, breath.getWidth() / (int) breathKnobs.size());

    // --- Texture panel ---
    texturePanel.setBounds(texRow);
    texRow.reduce(10, 10);
    texRow.removeFromTop(20);
    auto texRow1 = texRow.removeFromTop(70);
    for (int i = 0; i < 5; ++i)
        textureKnobs[(size_t) i]->setBounds(texRow1.removeFromLeft(texRow1.getWidth() / (5 - i)));
    auto texRow2 = texRow.removeFromTop(70);
    for (int i = 5; i < 10; ++i)
        textureKnobs[(size_t) i]->setBounds(texRow2.removeFromLeft(texRow2.getWidth() / (10 - i)));

    // --- Global/FX panel ---
    globalPanel.setBounds(fxRow);
    fxRow.reduce(10, 10);
    fxRow.removeFromTop(20);
    layoutKnobRow(fxRow, fxKnobs, fxRow.getWidth() / (int) fxKnobs.size());
}

void PlayTab::paint(juce::Graphics& g) { g.fillAll(Palette::bgDeep); }

} // namespace raiu::ui
