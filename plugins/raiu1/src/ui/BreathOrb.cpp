#include "BreathOrb.h"
#include "Palette.h"

namespace raiu::ui {

BreathOrb::BreathOrb(juce::AudioProcessorValueTreeState& state) : apvts(state) {
    startTimerHz(30);
}
BreathOrb::~BreathOrb() { stopTimer(); }

void BreathOrb::timerCallback() {
    t += 1.0 / 30.0;
    auto smooth = [](float& s, float target) { s += 0.08f * (target - s); };
    smooth(breathS, apvts.getRawParameterValue("breath")->load());
    smooth(velS, apvts.getRawParameterValue("vel_to_texture")->load());
    smooth(swellS, apvts.getRawParameterValue("swell")->load());
    repaint();
}

void BreathOrb::paint(juce::Graphics& g) {
    auto bounds = getLocalBounds().toFloat();
    const float cx = bounds.getCentreX(), cy = bounds.getCentreY();
    const float maxR = juce::jmin(bounds.getWidth(), bounds.getHeight()) * 0.5f - 4.0f;
    const float breathe = 1.0f + 0.03f * (float) std::sin(t * (0.6 + 1.2 * breathS));

    struct Ring {
        float value;
        juce::Colour colour;
        float baseFrac;
    };
    const Ring rings[3] = {
        {swellS, Palette::interaction.brighter(0.15f), 0.42f},
        {velS, Palette::texture, 0.66f},
        {breathS, Palette::nature, 0.92f},
    };

    // 中心コア
    juce::ColourGradient core(Palette::interaction.withAlpha(0.5f * breathe - 0.15f), cx, cy,
                              juce::Colours::transparentBlack, cx, cy, true);
    core.addColour(0.6, Palette::interaction.withAlpha(0.18f));
    g.setGradientFill(core);
    g.fillEllipse(cx - maxR * 0.3f * breathe, cy - maxR * 0.3f * breathe, maxR * 0.6f * breathe,
                 maxR * 0.6f * breathe);

    for (const auto& r : rings) {
        float radius = maxR * r.baseFrac * breathe;
        float alpha = 0.12f + 0.55f * r.value;
        g.setColour(r.colour.withAlpha(alpha));
        g.drawEllipse(cx - radius, cy - radius, radius * 2.0f, radius * 2.0f,
                     1.6f + 3.0f * r.value);

        // 値に応じた弧のハイライト（0時位置から時計回り）
        if (r.value > 0.02f) {
            juce::Path arc;
            float sweep = juce::MathConstants<float>::twoPi * r.value;
            arc.addCentredArc(cx, cy, radius, radius, 0.0f, 0.0f, sweep, true);
            g.setColour(r.colour.withAlpha(0.9f));
            g.strokePath(arc, juce::PathStrokeType(2.2f, juce::PathStrokeType::curved,
                                                   juce::PathStrokeType::rounded));
        }
    }

    g.setColour(Palette::textFaint);
    g.setFont(juce::FontOptions(9.5f));
    juce::Rectangle<float> labelArea(cx - maxR, bounds.getBottom() - 14.0f, maxR * 2.0f, 12.0f);
    g.drawText("BREATH", labelArea, juce::Justification::centred);
}

} // namespace raiu::ui
