#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include "Palette.h"

namespace raiu::ui {

// 丸角パネル背景 + タイトル + アクセントカラーの下線。セクションを視覚的にグループ化する。
class SectionPanel : public juce::Component {
public:
    SectionPanel(juce::String titleText, juce::Colour accentColour)
        : title(std::move(titleText)), accent(accentColour) {
        setInterceptsMouseClicks(false, true);
    }

    void paint(juce::Graphics& g) override {
        auto bounds = getLocalBounds().toFloat();
        g.setColour(Palette::panel.withAlpha(0.6f));
        g.fillRoundedRectangle(bounds, 12.0f);
        g.setColour(Palette::hairline.withAlpha(0.7f));
        g.drawRoundedRectangle(bounds.reduced(0.5f), 12.0f, 1.0f);

        if (title.isNotEmpty()) {
            auto titleArea = bounds.removeFromTop(24.0f).reduced(12.0f, 4.0f);
            g.setColour(accent);
            g.fillRoundedRectangle(titleArea.removeFromLeft(3.0f).withHeight(14.0f), 1.5f);
            g.setColour(Palette::textMuted);
            g.setFont(juce::FontOptions(12.0f, juce::Font::bold).withKerningFactor(0.08f));
            g.drawText(title.toUpperCase(), titleArea.withTrimmedLeft(8.0f),
                      juce::Justification::centredLeft);
        }
    }

private:
    juce::String title;
    juce::Colour accent;
};

} // namespace raiu::ui
