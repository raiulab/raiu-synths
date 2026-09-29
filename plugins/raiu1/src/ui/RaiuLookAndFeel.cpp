#include "RaiuLookAndFeel.h"
#include "Palette.h"

namespace raiu::ui {

RaiuLookAndFeel::RaiuLookAndFeel() {
    setColour(juce::ResizableWindow::backgroundColourId, Palette::bgDeep);
    setColour(juce::Slider::textBoxTextColourId, Palette::textPrimary);
    setColour(juce::Slider::textBoxBackgroundColourId, juce::Colours::transparentBlack);
    setColour(juce::Slider::textBoxOutlineColourId, juce::Colours::transparentBlack);
    setColour(juce::Label::textColourId, Palette::textMuted);
    setColour(juce::ComboBox::backgroundColourId, Palette::panel);
    setColour(juce::ComboBox::textColourId, Palette::textPrimary);
    setColour(juce::ComboBox::outlineColourId, Palette::hairline);
    setColour(juce::PopupMenu::backgroundColourId, Palette::panelRaised);
    setColour(juce::PopupMenu::textColourId, Palette::textPrimary);
    setColour(juce::PopupMenu::highlightedBackgroundColourId, Palette::fm.withAlpha(0.3f));
    setColour(juce::TextButton::buttonColourId, Palette::panel);
    setColour(juce::TextButton::buttonOnColourId, Palette::fm.withAlpha(0.35f));
    setColour(juce::TextButton::textColourOffId, Palette::textMuted);
    setColour(juce::TextButton::textColourOnId, Palette::textPrimary);
    setColour(juce::ToggleButton::textColourId, Palette::textMuted);
    setColour(juce::ToggleButton::tickColourId, Palette::fm);
    setColour(juce::ToggleButton::tickDisabledColourId, Palette::hairline);
    setColour(juce::TabbedComponent::backgroundColourId, juce::Colours::transparentBlack);
    setColour(juce::TabbedButtonBar::tabOutlineColourId, juce::Colours::transparentBlack);
    setColour(juce::TabbedButtonBar::frontOutlineColourId, juce::Colours::transparentBlack);
}

void RaiuLookAndFeel::drawRotarySlider(juce::Graphics& g, int x, int y, int width, int height,
                                       float pos, float startAngle, float endAngle,
                                       juce::Slider& slider) {
    auto bounds = juce::Rectangle<float>((float) x, (float) y, (float) width, (float) height)
                      .reduced(4.0f);
    const float cx = bounds.getCentreX(), cy = bounds.getCentreY();
    const float radius = juce::jmin(bounds.getWidth(), bounds.getHeight()) * 0.5f;
    const float angle = startAngle + pos * (endAngle - startAngle);

    juce::Colour accent = slider.findColour(juce::Slider::rotarySliderFillColourId, true);
    if (accent == juce::Colours::transparentBlack) accent = Palette::fm;
    const bool enabled = slider.isEnabled();
    const float alphaMul = enabled ? 1.0f : 0.35f;

    // 中心オーブ（放射グラデーション、奥行き感）
    juce::ColourGradient orbGrad(Palette::panelRaised.brighter(0.06f), cx, cy,
                                 Palette::bgVoid, cx, cy + radius * 0.95f, true);
    g.setGradientFill(orbGrad);
    g.fillEllipse(cx - radius * 0.72f, cy - radius * 0.72f, radius * 1.44f, radius * 1.44f);

    // トラック（全域の薄い弧）
    juce::Path track;
    track.addCentredArc(cx, cy, radius * 0.92f, radius * 0.92f, 0.0f, startAngle, endAngle, true);
    g.setColour(Palette::hairline.withAlpha(0.8f));
    g.strokePath(track, juce::PathStrokeType(2.6f, juce::PathStrokeType::curved,
                                             juce::PathStrokeType::rounded));

    // 値の弧: 淡いグローを下敷きに、上に本体を重ねる
    juce::Path valueArc;
    valueArc.addCentredArc(cx, cy, radius * 0.92f, radius * 0.92f, 0.0f, startAngle, angle, true);
    g.setColour(accent.withAlpha(0.28f * alphaMul));
    g.strokePath(valueArc, juce::PathStrokeType(7.0f, juce::PathStrokeType::curved,
                                                juce::PathStrokeType::rounded));
    g.setColour(accent.withAlpha(0.95f * alphaMul));
    g.strokePath(valueArc, juce::PathStrokeType(2.6f, juce::PathStrokeType::curved,
                                                juce::PathStrokeType::rounded));

    // 指針: 中心から先端の小さな輝点へ
    const float px = cx + std::sin(angle) * radius * 0.68f;
    const float py = cy - std::cos(angle) * radius * 0.68f;
    g.setColour(Palette::textPrimary.withAlpha(0.85f * alphaMul));
    g.drawLine(cx + std::sin(angle) * radius * 0.2f, cy - std::cos(angle) * radius * 0.2f,
              px, py, 2.0f);
    g.setColour(accent.withAlpha(alphaMul));
    g.fillEllipse(px - 3.0f, py - 3.0f, 6.0f, 6.0f);
    g.setColour(accent.withAlpha(0.35f * alphaMul));
    g.fillEllipse(px - 5.5f, py - 5.5f, 11.0f, 11.0f);
}

void RaiuLookAndFeel::drawComboBox(juce::Graphics& g, int width, int height, bool,
                                   int, int, int, int, juce::ComboBox& box) {
    auto bounds = juce::Rectangle<float>(0, 0, (float) width, (float) height).reduced(1.0f);
    g.setColour(Palette::panel);
    g.fillRoundedRectangle(bounds, 7.0f);
    g.setColour(Palette::hairline);
    g.drawRoundedRectangle(bounds, 7.0f, 1.2f);

    auto arrowZone = bounds.removeFromRight(24.0f);
    juce::Path arrow;
    float ax = arrowZone.getCentreX(), ay = arrowZone.getCentreY();
    arrow.addTriangle(ax - 4.5f, ay - 2.5f, ax + 4.5f, ay - 2.5f, ax, ay + 3.5f);
    g.setColour(box.findColour(juce::ComboBox::textColourId).withAlpha(0.7f));
    g.fillPath(arrow);
}

void RaiuLookAndFeel::drawButtonBackground(juce::Graphics& g, juce::Button& button,
                                          const juce::Colour&, bool highlighted, bool down) {
    auto bounds = button.getLocalBounds().toFloat().reduced(1.0f);
    juce::Colour base = button.getToggleState()
                            ? button.findColour(juce::TextButton::buttonOnColourId)
                            : Palette::panel;
    if (down) base = base.brighter(0.1f);
    else if (highlighted) base = base.brighter(0.05f);
    g.setColour(base);
    g.fillRoundedRectangle(bounds, 7.0f);
    g.setColour(button.getToggleState() ? Palette::fm.withAlpha(0.7f) : Palette::hairline);
    g.drawRoundedRectangle(bounds, 7.0f, 1.2f);
}

void RaiuLookAndFeel::drawToggleButton(juce::Graphics& g, juce::ToggleButton& button,
                                      bool, bool) {
    auto bounds = button.getLocalBounds().toFloat();
    const float boxSize = 16.0f;
    auto box = juce::Rectangle<float>(bounds.getX(), bounds.getCentreY() - boxSize * 0.5f,
                                      boxSize, boxSize);
    g.setColour(Palette::panel);
    g.fillRoundedRectangle(box, 4.0f);
    g.setColour(button.getToggleState() ? Palette::fm : Palette::hairline);
    g.drawRoundedRectangle(box, 4.0f, 1.4f);
    if (button.getToggleState()) {
        g.setColour(Palette::fm);
        g.fillRoundedRectangle(box.reduced(3.5f), 2.0f);
    }
    g.setColour(button.findColour(juce::ToggleButton::textColourId));
    g.setFont(juce::FontOptions(13.0f));
    g.drawText(button.getButtonText(), bounds.withTrimmedLeft(boxSize + 8.0f),
              juce::Justification::centredLeft);
}

juce::Font RaiuLookAndFeel::getLabelFont(juce::Label&) {
    return juce::FontOptions(11.5f).withKerningFactor(0.03f);
}
juce::Font RaiuLookAndFeel::getComboBoxFont(juce::ComboBox&) { return juce::FontOptions(13.0f); }
juce::Font RaiuLookAndFeel::getTextButtonFont(juce::TextButton&, int) {
    return juce::FontOptions(12.5f);
}

void RaiuLookAndFeel::drawTabButton(juce::TabBarButton& button, juce::Graphics& g,
                                   bool isMouseOver, bool) {
    auto bounds = button.getActiveArea().toFloat();
    const bool active = button.getToggleState();
    if (active) {
        g.setColour(Palette::panelRaised);
        g.fillRoundedRectangle(bounds.reduced(2.0f), 8.0f);
        g.setColour(Palette::fm.withAlpha(0.6f));
        g.fillRoundedRectangle(bounds.reduced(2.0f).removeFromBottom(2.5f), 1.2f);
    } else if (isMouseOver) {
        g.setColour(Palette::panel.withAlpha(0.6f));
        g.fillRoundedRectangle(bounds.reduced(2.0f), 8.0f);
    }
    g.setColour(active ? Palette::textPrimary : Palette::textMuted);
    g.setFont(juce::FontOptions(13.5f, active ? juce::Font::bold : juce::Font::plain));
    g.drawText(button.getButtonText(), bounds, juce::Justification::centred);
}

juce::Font RaiuLookAndFeel::getTabButtonFont(juce::TabBarButton&, float height) {
    return juce::FontOptions(juce::jmin(15.0f, height * 0.5f));
}

void RaiuLookAndFeel::drawTabbedButtonBarBackground(juce::TabbedButtonBar&, juce::Graphics&) {}

} // namespace raiu::ui
