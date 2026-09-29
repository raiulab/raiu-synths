#include "PluginEditor.h"
#include "Presets.h"
#include "ui/Palette.h"

namespace raiu {

Raiu1Editor::Raiu1Editor(Raiu1Processor& p) : AudioProcessorEditor(p), proc(p) {
    setLookAndFeel(&lookAndFeel);

    titleLabel.setText("RAIU-1  BREEZE FM", juce::dontSendNotification);
    titleLabel.setFont(juce::FontOptions(17.0f, juce::Font::bold).withKerningFactor(0.12f));
    titleLabel.setColour(juce::Label::textColourId, ui::Palette::textPrimary);
    addAndMakeVisible(titleLabel);

    const auto& presets = presets::factoryPresets();
    for (int i = 0; i < (int) presets.size(); ++i)
        presetBox.addItem(presets[(size_t) i].name, i + 1);
    presetBox.setTextWhenNothingSelected("Presets...");
    presetBox.onChange = [this] {
        int idx = presetBox.getSelectedId() - 1;
        if (idx >= 0) proc.setCurrentProgram(idx);
    };
    addAndMakeVisible(presetBox);

    tabs.setOutline(0);
    tabs.setIndent(0);
    tabs.addTab("PLAY", juce::Colours::transparentBlack, new ui::PlayTab(proc.apvts), true);
    tabs.addTab("EDIT", juce::Colours::transparentBlack, new ui::EditTab(proc.apvts), true);
    addAndMakeVisible(tabs);

    setResizable(false, false);
    setSize(1000, 780);
}

Raiu1Editor::~Raiu1Editor() { setLookAndFeel(nullptr); }

void Raiu1Editor::resized() {
    auto b = getLocalBounds();
    auto header = b.removeFromTop(42);
    titleLabel.setBounds(header.removeFromLeft(280).withTrimmedLeft(14));
    presetBox.setBounds(header.removeFromRight(200).reduced(4));
    tabs.setBounds(b);
}

void Raiu1Editor::paint(juce::Graphics& g) {
    juce::ColourGradient bg(ui::Palette::bgVoid, 0, 0, ui::Palette::bgDeep, 0,
                            (float) getHeight(), false);
    g.setGradientFill(bg);
    g.fillAll();
    g.setColour(ui::Palette::hairline.withAlpha(0.6f));
    g.drawHorizontalLine(41, 0.0f, (float) getWidth());
}

} // namespace raiu
