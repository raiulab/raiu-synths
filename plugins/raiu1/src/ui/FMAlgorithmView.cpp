#include "FMAlgorithmView.h"
#include "Palette.h"

namespace raiu::ui {

const std::array<FMAlgorithmView::Layout, 8>& FMAlgorithmView::layouts() {
    static const std::array<Layout, 8> data{{
        // 1: 4->3->2->1（直列）
        {{{{0.5f, 0.86f}, {0.5f, 0.62f}, {0.5f, 0.38f}, {0.5f, 0.14f}}},
         {{{3, 2}, {2, 1}, {1, 0}, {-1, -1}}}, 3, 0b0001, "Serial x4 - hard, edgy"},
        // 2: (3+4)->2->1
        {{{{0.5f, 0.86f}, {0.5f, 0.58f}, {0.28f, 0.16f}, {0.72f, 0.16f}}},
         {{{2, 1}, {3, 1}, {1, 0}, {-1, -1}}}, 3, 0b0001, "Dual mod into serial"},
        // 3: (2->1)+(4->3)
        {{{{0.26f, 0.78f}, {0.26f, 0.24f}, {0.74f, 0.78f}, {0.74f, 0.24f}}},
         {{{1, 0}, {3, 2}, {-1, -1}, {-1, -1}}}, 2, 0b0101, "Two pairs - electric piano"},
        // 4: (2+3+4)->1
        {{{{0.5f, 0.82f}, {0.2f, 0.2f}, {0.5f, 0.2f}, {0.8f, 0.2f}}},
         {{{1, 0}, {2, 0}, {3, 0}, {-1, -1}}}, 3, 0b0001, "Triple stack - dense bell"},
        // 5: (2->1)+3+4
        {{{{0.2f, 0.78f}, {0.2f, 0.26f}, {0.58f, 0.52f}, {0.86f, 0.52f}}},
         {{{1, 0}, {-1, -1}, {-1, -1}, {-1, -1}}}, 1, 0b1101, "One pair + two tones"},
        // 6: 4->(1,2,3)
        {{{{0.2f, 0.78f}, {0.5f, 0.78f}, {0.8f, 0.78f}, {0.5f, 0.2f}}},
         {{{3, 0}, {3, 1}, {3, 2}, {-1, -1}}}, 3, 0b0111, "One mod, three voices"},
        // 7: (4->3->2)+1
        {{{{0.2f, 0.5f}, {0.76f, 0.82f}, {0.76f, 0.52f}, {0.76f, 0.22f}}},
         {{{3, 2}, {2, 1}, {-1, -1}, {-1, -1}}}, 2, 0b0011, "Chain + pure tone"},
        // 8: 全並列
        {{{{0.14f, 0.5f}, {0.38f, 0.5f}, {0.62f, 0.5f}, {0.86f, 0.5f}}},
         {{{-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}}}, 0, 0b1111, "All parallel - organ"},
    }};
    return data;
}

FMAlgorithmView::FMAlgorithmView(juce::AudioProcessorValueTreeState& state) : apvts(state) {
    startTimerHz(30);
}
FMAlgorithmView::~FMAlgorithmView() { stopTimer(); }

void FMAlgorithmView::timerCallback() {
    bool changed = false;
    for (int i = 0; i < 4; ++i) {
        juce::String id = "op" + juce::String(i + 1) + "_level";
        float target = apvts.getRawParameterValue(id)->load();
        float next = smoothedLevel[(size_t) i] + 0.25f * (target - smoothedLevel[(size_t) i]);
        if (std::abs(next - smoothedLevel[(size_t) i]) > 0.001f) changed = true;
        smoothedLevel[(size_t) i] = next;
    }
    float fbTarget = apvts.getRawParameterValue("feedback")->load();
    smoothedFeedback += 0.25f * (fbTarget - smoothedFeedback);
    if (changed) repaint();
}

void FMAlgorithmView::mouseUp(const juce::MouseEvent& e) {
    auto* p = apvts.getParameter("algorithm");
    if (p == nullptr) return;
    int current = (int) std::round(p->convertFrom0to1(p->getValue()));
    int next = e.mods.isRightButtonDown() ? (current + 7) % 8 : (current + 1) % 8;
    p->setValueNotifyingHost(p->convertTo0to1((float) next));
}

void FMAlgorithmView::paint(juce::Graphics& g) {
    auto bounds = getLocalBounds().toFloat().reduced(6.0f);
    int algoIndex = (int) apvts.getRawParameterValue("algorithm")->load();
    algoIndex = juce::jlimit(0, 7, algoIndex);
    const auto& layout = layouts()[(size_t) algoIndex];

    const float nodeR = juce::jmin(bounds.getWidth(), bounds.getHeight()) * 0.085f;
    std::array<juce::Point<float>, 4> pts;
    for (int i = 0; i < 4; ++i)
        pts[(size_t) i] = {bounds.getX() + layout.pos[(size_t) i].x * bounds.getWidth(),
                          bounds.getY() + layout.pos[(size_t) i].y * bounds.getHeight()};

    // 矢印（変調の流れ）
    for (int a = 0; a < layout.numArrows; ++a) {
        auto [from, to] = layout.arrows[(size_t) a];
        if (from < 0) continue;
        auto p0 = pts[(size_t) from], p1 = pts[(size_t) to];
        auto dir = (p1 - p0);
        float len = dir.getDistanceFromOrigin();
        if (len < 1.0f) continue;
        dir = dir / len;
        auto start = p0 + dir * (nodeR + 2.0f);
        auto end = p1 - dir * (nodeR + 6.0f);

        float level = smoothedLevel[(size_t) from];
        g.setColour(Palette::fm.withAlpha(0.15f + 0.35f * level));
        g.drawLine({start, end}, 1.5f + 2.0f * level);

        juce::Path arrowHead;
        float ah = 5.0f + 2.0f * level;
        auto perp = juce::Point<float>(-dir.y, dir.x);
        auto tip = end;
        arrowHead.addTriangle(tip, tip - dir * ah + perp * ah * 0.6f,
                              tip - dir * ah - perp * ah * 0.6f);
        g.setColour(Palette::fm.withAlpha(0.4f + 0.5f * level));
        g.fillPath(arrowHead);
    }

    // フィードバック（op4の自己ループ、算出対象がop4を含むアルゴリズムのみ意味を持つ）
    if (smoothedFeedback > 0.02f) {
        auto p3 = pts[3];
        juce::Path loop;
        loop.addCentredArc(p3.x + nodeR * 0.9f, p3.y - nodeR * 0.9f, nodeR * 0.9f, nodeR * 0.9f,
                           0.0f, 0.6f, 5.6f, true);
        g.setColour(Palette::fm.withAlpha(0.25f + 0.5f * smoothedFeedback));
        g.strokePath(loop, juce::PathStrokeType(1.6f));
    }

    // ノード
    for (int i = 0; i < 4; ++i) {
        auto pt = pts[(size_t) i];
        bool isCarrier = (layout.carrierMask & (1u << i)) != 0;
        float level = smoothedLevel[(size_t) i];

        if (isCarrier) {
            g.setColour(Palette::fm.withAlpha(0.12f + 0.22f * level));
            g.fillEllipse(pt.x - nodeR * 1.6f, pt.y - nodeR * 1.6f, nodeR * 3.2f, nodeR * 3.2f);
        }
        juce::Colour fill = isCarrier
                                ? Palette::fm.withAlpha(0.35f + 0.55f * level)
                                : Palette::panelRaised.brighter(0.04f * level);
        g.setColour(fill);
        g.fillEllipse(pt.x - nodeR, pt.y - nodeR, nodeR * 2.0f, nodeR * 2.0f);
        g.setColour(isCarrier ? Palette::fm : Palette::hairline);
        g.drawEllipse(pt.x - nodeR, pt.y - nodeR, nodeR * 2.0f, nodeR * 2.0f, 1.4f);

        g.setColour(Palette::textPrimary);
        g.setFont(juce::FontOptions(nodeR * 0.95f, juce::Font::bold));
        g.drawText(juce::String(i + 1),
                  juce::Rectangle<float>(pt.x - nodeR, pt.y - nodeR, nodeR * 2.0f, nodeR * 2.0f),
                  juce::Justification::centred);
    }

    g.setColour(Palette::textFaint);
    g.setFont(juce::FontOptions(10.5f));
    g.drawText(layout.description, bounds.removeFromBottom(14.0f),
              juce::Justification::centred);
}

} // namespace raiu::ui
