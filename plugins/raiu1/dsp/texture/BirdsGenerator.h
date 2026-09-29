#pragma once

#include "TextureCommon.h"
#include "../SineTable.h"

namespace raiu {

// 鳥: ランダムなタイミングで鳴く「さえずり」= 周波数が弧を描いてスイープする正弦チャープ
class BirdsGenerator {
public:
    void setSampleRate(double sr) noexcept {
        sampleRate = sr;
        trigger.setRate(1.2f, sr);
    }

    float next() noexcept {
        if (remaining <= 0 && trigger.fire(rng)) spawnChirp(false);

        if (remaining <= 0) return 0.0f;

        // 周波数軌道: 開始→頂点→終了の2次カーブ（さえずり特有の「ピュルル」）
        float t = 1.0f - (float) remaining / (float) duration;  // 0..1
        float hz = quadratic(f0, fPeak, f1, t);
        phase += hz / (float) sampleRate;
        phase -= std::floor(phase);

        // 振幅: sin^2窓（0→1→0の山型）+ 細かいトレモロ（喉の震え）
        // ※ 以前は位相オフセットが誤っており単調増加のまま最大振幅で
        //   打ち切られていた（さえずり終わりに毎回クリックが出るバグ）
        float window = SineTable::lookup(0.5f * t);
        window *= window;
        float trill = 0.75f + 0.25f * SineTable::lookup(trillPhase);
        trillPhase += trillHz / (float) sampleRate;
        trillPhase -= std::floor(trillPhase);

        --remaining;
        if (remaining <= 0 && followUp) spawnChirp(true);  // 2連さえずり

        return amp * window * trill * SineTable::lookup(phase);
    }

private:
    static float quadratic(float a, float b, float c, float t) noexcept {
        // t=0でa、t=0.5でb、t=1でc を通る2次補間
        float u = 1.0f - t;
        return a * u * u + 2.0f * b * u * t + c * t * t;
    }

    void spawnChirp(bool isFollowUp) noexcept {
        f0 = 2000.0f + 2000.0f * rng.uniform();
        fPeak = f0 * (1.15f + 0.55f * rng.uniform());
        f1 = f0 * (0.9f + 0.3f * rng.uniform());
        duration = (int) ((0.06f + 0.19f * rng.uniform()) * (float) sampleRate);
        remaining = duration;
        amp = 0.3f + 0.25f * rng.uniform();
        trillHz = 15.0f + 25.0f * rng.uniform();
        phase = 0.0f;
        followUp = !isFollowUp && rng.uniform() < 0.3f;
    }

    double sampleRate = 48000.0;
    tex::Rng rng{20250131u};
    tex::PoissonTrigger trigger;
    float phase = 0, trillPhase = 0, trillHz = 20.0f;
    float f0 = 3000, fPeak = 4000, f1 = 3000, amp = 0;
    int duration = 1, remaining = 0;
    bool followUp = false;
};

} // namespace raiu
