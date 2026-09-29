#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

namespace raiu::ui {

// 有機的・未来的なダークUIのための共通ルック&フィール。
// ノブはグロー付きの円弧（トラック+値+ソフトグロー+中心オーブ）で描く。
// アクセントカラーは各Sliderの rotarySliderFillColourId で個別に設定する。
class RaiuLookAndFeel : public juce::LookAndFeel_V4 {
public:
    RaiuLookAndFeel();

    void drawRotarySlider(juce::Graphics&, int x, int y, int width, int height,
                          float sliderPosProportional, float rotaryStartAngle,
                          float rotaryEndAngle, juce::Slider&) override;

    void drawComboBox(juce::Graphics&, int width, int height, bool isButtonDown,
                      int buttonX, int buttonY, int buttonW, int buttonH,
                      juce::ComboBox&) override;

    void drawButtonBackground(juce::Graphics&, juce::Button&, const juce::Colour& bg,
                              bool highlighted, bool down) override;

    void drawToggleButton(juce::Graphics&, juce::ToggleButton&, bool highlighted,
                          bool down) override;

    juce::Font getLabelFont(juce::Label&) override;
    juce::Font getComboBoxFont(juce::ComboBox&) override;
    juce::Font getTextButtonFont(juce::TextButton&, int buttonHeight) override;

    void drawTabButton(juce::TabBarButton&, juce::Graphics&, bool isMouseOver,
                       bool isMouseDown) override;
    juce::Font getTabButtonFont(juce::TabBarButton&, float height) override;
    void drawTabbedButtonBarBackground(juce::TabbedButtonBar&, juce::Graphics&) override;
};

} // namespace raiu::ui
