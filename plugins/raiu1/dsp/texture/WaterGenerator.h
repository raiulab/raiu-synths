#pragma once

#include "TextureCommon.h"
#include "../SineTable.h"
#include <array>

namespace raiu {

// 水: 上昇チャープの「泡」（ポアソン発生）+ 低域の「せせらぎ」
class WaterGenerator {
public:
    void setSampleRate(double sr) noexcept {
        sampleRate = sr;
        bubbleTrigger.setRate(7.0f, sr);
        babbleBand.set(550.0f, 1.5f, sr);
        babbleMod.setRate(3.0f, sr, rng);
        dcBlock.setCutoff(20.0f, sr);
    }

    float next() noexcept {
        // 泡: 開始周波数からピッチが指数的に上がる正弦チャープ（物理的な泡の共鳴）
        if (bubbleTrigger.fire(rng)) spawnBubble();
        float bubbles = 0.0f;
        for (auto& b : bubbleSlots) {
            if (b.amp <= 1e-4f) continue;
            b.phase += b.hz / (float) sampleRate;
            b.phase -= std::floor(b.phase);
            bubbles += b.amp * SineTable::lookup(b.phase);
            b.hz *= b.rise;        // ピッチ上昇
            b.amp *= b.decay;      // 減衰
        }

        // せせらぎ: 中低域バンドパスノイズを数Hzのゆらぎで振幅変調
        float m = babbleMod.next(rng);
        float babble = babbleBand.process(rng.bipolar()) * (0.3f + 0.7f * m * m);

        return dcBlock.process(bubbles * 0.5f + babble * 0.55f);
    }

private:
    struct Bubble {
        float phase = 0, hz = 0, amp = 0, rise = 1, decay = 1;
    };

    void spawnBubble() noexcept {
        for (auto& b : bubbleSlots) {
            if (b.amp > 1e-4f) continue;
            b.phase = 0;
            b.hz = 400.0f + 900.0f * rng.uniform();
            b.amp = 0.25f + 0.3f * rng.uniform();
            // 約60〜100msで減衰し、その間にピッチが1.5倍ほど上がる
            float lifeSec = 0.06f + 0.04f * rng.uniform();
            b.decay = std::exp(-1.0f / (lifeSec * 0.3f * (float) sampleRate));
            b.rise = std::exp(0.4f / (lifeSec * (float) sampleRate));
            return;
        }
    }

    double sampleRate = 48000.0;
    tex::Rng rng{47u};
    tex::PoissonTrigger bubbleTrigger;
    std::array<Bubble, 8> bubbleSlots;
    tex::BiquadBP babbleBand;
    tex::SlowRandom babbleMod;
    tex::OnePoleHP dcBlock;
};

} // namespace raiu
