#pragma once

#include "PluginProcessor.h"
#include "ui/RaiuLookAndFeel.h"
#include "ui/PlayTab.h"
#include "ui/EditTab.h"

namespace raiu {

// M6: 有機的・未来的なカスタムUI。PLAY（直感的なヒーロー画面）と
// EDIT（FMオペレーターの詳細編集）の2タブ構成。
class Raiu1Editor : public juce::AudioProcessorEditor {
public:
    explicit Raiu1Editor(Raiu1Processor& p);
    ~Raiu1Editor() override;

    void paint(juce::Graphics& g) override;
    void resized() override;

private:
    Raiu1Processor& proc;
    ui::RaiuLookAndFeel lookAndFeel;

    juce::ComboBox presetBox;
    juce::Label titleLabel;
    juce::TabbedComponent tabs{juce::TabbedButtonBar::TabsAtTop};

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(Raiu1Editor)
};

} // namespace raiu
