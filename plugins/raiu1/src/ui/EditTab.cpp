#include "EditTab.h"
#include "../Params.h"

namespace raiu::ui {

EditTab::EditTab(juce::AudioProcessorValueTreeState& apvts) {
    static const std::pair<const char*, const char*> opParams[] = {
        {"ratio", "Ratio"},   {"detune", "Detune"},   {"fixedhz", "FixHz"},
        {"level", "Level"},   {"attack", "A"},        {"decay", "D"},
        {"sustain", "S"},     {"release", "R"},       {"velsens", "Vel"},
        {"keyscale", "KeySc"}};

    for (int op = 0; op < 4; ++op) {
        addAndMakeVisible(opPanels[op]);
        for (const auto& [id, name] : opParams) {
            auto k = std::make_unique<Knob>(apvts, params::opId(op + 1, id), name, Palette::fm);
            addAndMakeVisible(*k);
            opKnobs[op].push_back(std::move(k));
        }
        fixedButtons[op].setButtonText("Fix");
        addAndMakeVisible(fixedButtons[op]);
        fixedAtts[op] = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(
            apvts, params::opId(op + 1, "fixed"), fixedButtons[op]);
    }
}

void EditTab::resized() {
    auto full = getLocalBounds().reduced(10);
    const int rowH = full.getHeight() / 4;
    for (int op = 0; op < 4; ++op) {
        auto row = full.removeFromTop(rowH).reduced(0, 4);
        opPanels[op].setBounds(row);
        row.reduce(12, 10);
        row.removeFromTop(20);
        auto fixArea = row.removeFromRight(56);
        fixedButtons[op].setBounds(fixArea.withSizeKeepingCentre(50, 24));
        layoutKnobRow(row, opKnobs[op], row.getWidth() / (int) opKnobs[op].size());
    }
}

void EditTab::paint(juce::Graphics& g) { g.fillAll(Palette::bgDeep); }

} // namespace raiu::ui
