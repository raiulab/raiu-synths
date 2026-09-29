#include "NatureOscPad.h"
#include "Palette.h"

namespace raiu::ui {

namespace {
constexpr int kNumModes = 6;
const char* kModeNames[kNumModes] = {"Off", "Wind", "Water", "Birds", "Insects", "Rain"};

void drawModeIcon(juce::Graphics& g, juce::Rectangle<float> box, int mode, juce::Colour colour) {
    auto b = box.reduced(box.getWidth() * 0.18f);
    g.setColour(colour);
    juce::Path p;
    switch (mode) {
        case 0:  // Off: 空の円
            g.setColour(colour.withAlpha(0.5f));
            g.drawEllipse(b, 1.4f);
            break;
        case 1:  // Wind: 3本の波線
            for (int i = 0; i < 3; ++i) {
                float y = b.getY() + b.getHeight() * (0.22f + 0.28f * (float) i);
                juce::Path wave;
                wave.startNewSubPath(b.getX(), y);
                wave.cubicTo(b.getX() + b.getWidth() * 0.33f, y - b.getHeight() * 0.16f,
                            b.getX() + b.getWidth() * 0.66f, y + b.getHeight() * 0.16f,
                            b.getRight() - b.getWidth() * 0.1f * (float) i, y);
                g.strokePath(wave, juce::PathStrokeType(1.4f));
            }
            break;
        case 2: {  // Water: 雫
            juce::Path drop;
            float cx = b.getCentreX();
            drop.startNewSubPath(cx, b.getY());
            drop.quadraticTo(b.getRight(), b.getBottom() * 0.72f + b.getY() * 0.28f, cx,
                             b.getBottom());
            drop.quadraticTo(b.getX(), b.getBottom() * 0.72f + b.getY() * 0.28f, cx, b.getY());
            g.fillPath(drop);
            break;
        }
        case 3: {  // Birds: 弧を描くM字
            juce::Path bird;
            float cy = b.getCentreY();
            bird.startNewSubPath(b.getX(), cy + b.getHeight() * 0.25f);
            bird.quadraticTo(b.getX() + b.getWidth() * 0.25f, b.getY(), b.getCentreX(),
                             cy + b.getHeight() * 0.15f);
            bird.quadraticTo(b.getRight() - b.getWidth() * 0.25f, b.getY(), b.getRight(),
                             cy + b.getHeight() * 0.25f);
            g.strokePath(bird, juce::PathStrokeType(1.6f, juce::PathStrokeType::curved,
                                                    juce::PathStrokeType::rounded));
            break;
        }
        case 4:  // Insects: 胴体+触角
            g.fillEllipse(b.getCentreX() - b.getWidth() * 0.16f, b.getCentreY() - b.getHeight() * 0.12f,
                         b.getWidth() * 0.32f, b.getHeight() * 0.32f);
            g.drawLine(b.getCentreX(), b.getCentreY() - b.getHeight() * 0.1f, b.getX(), b.getY(), 1.2f);
            g.drawLine(b.getCentreX(), b.getCentreY() - b.getHeight() * 0.1f, b.getRight(), b.getY(), 1.2f);
            break;
        case 5:  // Rain: 斜めの雫3本
            for (int i = 0; i < 3; ++i) {
                float x = b.getX() + b.getWidth() * (0.15f + 0.35f * (float) i);
                g.drawLine(x, b.getY(), x - b.getWidth() * 0.12f, b.getBottom(), 1.6f);
            }
            break;
        default: break;
    }
}
} // namespace

NatureOscPad::NatureOscPad(juce::AudioProcessorValueTreeState& state) : apvts(state) {
    startTimerHz(30);
}
NatureOscPad::~NatureOscPad() { stopTimer(); }

juce::Rectangle<int> NatureOscPad::buttonBounds(int index) const {
    auto row = getLocalBounds().removeFromTop(40);
    int w = row.getWidth() / kNumModes;
    return row.withX(row.getX() + w * index).withWidth(w).reduced(3);
}

void NatureOscPad::resized() {
    visualArea = getLocalBounds().withTrimmedTop(46).reduced(2);
}

void NatureOscPad::mouseUp(const juce::MouseEvent& e) {
    for (int i = 0; i < kNumModes; ++i) {
        if (buttonBounds(i).contains(e.getPosition())) {
            if (auto* p = apvts.getParameter("nature_type"))
                p->setValueNotifyingHost(p->convertTo0to1((float) i));
            return;
        }
    }
}

void NatureOscPad::timerCallback() {
    t += 1.0 / 30.0;
    int mode = (int) apvts.getRawParameterValue("nature_type")->load();
    if (mode != lastMode) {
        particles.clear();
        lastMode = mode;
    }
    spawnAndUpdateParticles();
    repaint();
}

void NatureOscPad::spawnAndUpdateParticles() {
    const float level = apvts.getRawParameterValue("nature_level")->load();
    const float area = (float) visualArea.getWidth() * (float) visualArea.getHeight();
    if (area <= 1.0f) return;

    // 寿命が尽きた粒子を除去し、生存中を更新
    for (auto& p : particles) {
        p.life -= 1.0f / 30.0f;
        p.x += p.vx;
        p.y += p.vy;
    }
    particles.erase(std::remove_if(particles.begin(), particles.end(),
                                   [](const Particle& p) { return p.life <= 0.0f; }),
                    particles.end());
    if (particles.size() > 40) return;

    auto spawnChance = [&](float base) { return rng.nextFloat() < base * (0.2f + level); };
    float x0 = (float) visualArea.getX(), y0 = (float) visualArea.getY();
    float w = (float) visualArea.getWidth(), h = (float) visualArea.getHeight();

    switch (lastMode) {
        case 1:  // Wind: 横に流れる粒
            if (spawnChance(0.5f))
                particles.push_back({x0, y0 + rng.nextFloat() * h, 2.0f + rng.nextFloat() * 2.0f,
                                    0.0f, 1.5f, 1.5f, rng.nextFloat() * 6.28f});
            break;
        case 2:  // Water: 上昇する泡
            if (spawnChance(0.35f))
                particles.push_back({x0 + rng.nextFloat() * w, y0 + h, 0.0f,
                                    -(1.0f + rng.nextFloat() * 1.5f), 1.8f, 1.8f,
                                    rng.nextFloat() * 6.28f});
            break;
        case 3:  // Birds: 短命のさえずり弧
            if (spawnChance(0.12f))
                particles.push_back({x0 + rng.nextFloat() * w, y0 + rng.nextFloat() * h * 0.6f,
                                    0.6f, -0.2f, 0.5f, 0.5f, 0.0f});
            break;
        case 4:  // Insects: その場で明滅
            if (particles.size() < 10 && spawnChance(0.3f))
                particles.push_back({x0 + rng.nextFloat() * w, y0 + rng.nextFloat() * h, 0.0f,
                                    0.0f, 2.5f, 2.5f, rng.nextFloat() * 6.28f});
            break;
        case 5:  // Rain: 落下する筋
            if (spawnChance(0.6f))
                particles.push_back({x0 + rng.nextFloat() * w, y0, -0.6f,
                                    4.0f + rng.nextFloat() * 2.0f, 1.0f, 1.0f, 0.0f});
            break;
        default: break;
    }
}

void NatureOscPad::paint(juce::Graphics& g) {
    for (int i = 0; i < kNumModes; ++i) {
        auto b = buttonBounds(i).toFloat();
        bool active = (i == lastMode);
        g.setColour(active ? Palette::nature.withAlpha(0.22f) : Palette::panel.withAlpha(0.7f));
        g.fillRoundedRectangle(b, 6.0f);
        g.setColour(active ? Palette::nature : Palette::hairline);
        g.drawRoundedRectangle(b, 6.0f, active ? 1.6f : 1.0f);

        float side = juce::jmin(b.getWidth() * 0.4f, 20.0f);
        auto iconBox = juce::Rectangle<float>(side, side)
                          .withCentre({b.getCentreX(), b.getY() + b.getHeight() * 0.36f});
        drawModeIcon(g, iconBox, i, active ? Palette::nature : Palette::textMuted);
        g.setColour(active ? Palette::textPrimary : Palette::textFaint);
        g.setFont(juce::FontOptions(9.5f));
        g.drawText(kModeNames[i], b.withTrimmedTop(b.getHeight() * 0.58f),
                  juce::Justification::centred);
    }

    // 有機的アニメーション: 背景の呼吸グロー + 粒子
    auto v = visualArea.toFloat();
    g.setColour(Palette::panel.withAlpha(0.5f));
    g.fillRoundedRectangle(v, 10.0f);
    g.setColour(Palette::hairline.withAlpha(0.6f));
    g.drawRoundedRectangle(v, 10.0f, 1.0f);

    juce::Graphics::ScopedSaveState clip(g);
    juce::Path clipPath;
    clipPath.addRoundedRectangle(v, 10.0f);
    g.reduceClipRegion(clipPath);

    if (lastMode <= 0) {
        float pulse = 0.5f + 0.5f * (float) std::sin(t * 0.8);
        g.setColour(Palette::nature.withAlpha(0.06f + 0.05f * pulse));
        float r = v.getHeight() * (0.6f + 0.15f * pulse);
        g.fillEllipse(v.getCentreX() - r, v.getCentreY() - r, r * 2.0f, r * 2.0f);
        return;
    }

    for (const auto& p : particles) {
        float alpha = juce::jlimit(0.0f, 1.0f, p.life / p.maxLife);
        g.setColour(Palette::nature.withAlpha(0.15f + 0.55f * alpha));
        switch (lastMode) {
            case 1: {  // wind: 短い流線
                g.drawLine(p.x, p.y, p.x - 10.0f, p.y + std::sin(p.phase + t * 3.0) * 2.0f, 1.6f);
                break;
            }
            case 2: {  // water: 円
                float r = 2.0f + 2.0f * (1.0f - alpha);
                g.drawEllipse(p.x - r, p.y - r, r * 2.0f, r * 2.0f, 1.2f);
                break;
            }
            case 3: {  // birds: 小さな弧
                juce::Path arc;
                arc.addCentredArc(p.x, p.y, 4.0f, 3.0f, 0.0f, 0.3f, 2.8f, true);
                g.strokePath(arc, juce::PathStrokeType(1.4f));
                break;
            }
            case 4: {  // insects: 明滅する点
                float r = 1.2f + 1.0f * (0.5f + 0.5f * (float) std::sin(t * 14.0 + p.phase));
                g.fillEllipse(p.x - r, p.y - r, r * 2.0f, r * 2.0f);
                break;
            }
            case 5: {  // rain: 斜線
                g.drawLine(p.x, p.y, p.x + p.vx * 1.5f, p.y - p.vy * 0.5f, 1.3f);
                break;
            }
            default: break;
        }
    }
}

} // namespace raiu::ui
