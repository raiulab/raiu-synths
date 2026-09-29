#pragma once

#include "TextureCommon.h"

namespace raiu {

// 風: フィルターノイズ + 超低速ランダム変調（突風）+ 高域の「葉ずれ」成分
class WindGenerator {
public:
    void setSampleRate(double sr) noexcept {
        sampleRate = sr;
        gust.setRate(0.18f, sr, rng);
        cutoffMod.setRate(0.25f, sr, rng);
        leavesMod.setRate(0.35f, sr, rng);
        leavesBand.set(3500.0f, 1.2f, sr);
        body.setCutoff(500.0f, sr);
        dcBlock.setCutoff(20.0f, sr);
    }

    float next() noexcept {
        float noise = rng.bipolar();

        // 胴体: カットオフが 200〜1500Hz でゆっくり動くLPノイズ
        float c = cutoffMod.next(rng);
        body.setCutoff(200.0f + 1300.0f * c * c, sampleRate);
        float g = gust.next(rng);
        float gustGain = 0.25f + 0.75f * g * g;  // 凪〜突風
        float bodyOut = body.process(noise) * gustGain;

        // 葉ずれ: 3.5kHz帯のノイズが別のゆらぎで揺れる（風が強いほど鳴る）
        float lv = leavesMod.next(rng);
        float leavesOut = leavesBand.process(noise) * lv * gustGain * 0.7f;

        return dcBlock.process(bodyOut * 0.9f + leavesOut);
    }

private:
    double sampleRate = 48000.0;
    tex::Rng rng{20260720u};
    tex::SlowRandom gust, cutoffMod, leavesMod;
    tex::BiquadBP leavesBand;
    tex::OnePoleLP body;
    tex::OnePoleHP dcBlock;
};

} // namespace raiu
