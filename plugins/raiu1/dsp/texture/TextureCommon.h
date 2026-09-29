#pragma once

#include <cmath>
#include <cstdint>

namespace raiu::tex {

// 軽量な擬似乱数（xorshift32）。リアルタイムスレッドで安全。
struct Rng {
    uint32_t state = 0x9E3779B9u;

    explicit Rng(uint32_t seed = 0x9E3779B9u) : state(seed == 0 ? 1u : seed) {}

    uint32_t nextU32() noexcept {
        state ^= state << 13;
        state ^= state >> 17;
        state ^= state << 5;
        return state;
    }
    // [0,1)
    float uniform() noexcept { return (float) (nextU32() >> 8) * (1.0f / 16777216.0f); }
    // [-1,1)
    float bipolar() noexcept { return uniform() * 2.0f - 1.0f; }
};

struct OnePoleLP {
    void setCutoff(float hz, double sr) noexcept {
        coef = 1.0f - std::exp(-2.0f * 3.14159265f * hz / (float) sr);
    }
    float process(float x) noexcept { return z += coef * (x - z); }
    float coef = 0.1f, z = 0.0f;
};

struct OnePoleHP {
    void setCutoff(float hz, double sr) noexcept { lp.setCutoff(hz, sr); }
    float process(float x) noexcept { return x - lp.process(x); }
    OnePoleLP lp;
};

// 2次バンドパス（定Qピークゲイン正規化）
struct BiquadBP {
    void set(float hz, float q, double sr) noexcept {
        float w = 2.0f * 3.14159265f * hz / (float) sr;
        float alpha = std::sin(w) / (2.0f * q);
        float a0 = 1.0f + alpha;
        b0 = alpha / a0;
        b2 = -alpha / a0;
        a1 = -2.0f * std::cos(w) / a0;
        a2 = (1.0f - alpha) / a0;
    }
    float process(float x) noexcept {
        float y = b0 * x + b2 * x2 - a1 * y1 - a2 * y2;
        x2 = x1; x1 = x;
        y2 = y1; y1 = y;
        return y;
    }
    float b0 = 0, b2 = 0, a1 = 0, a2 = 0;
    float x1 = 0, x2 = 0, y1 = 0, y2 = 0;
};

// 2次ローパス（RBJ、レゾナンス可変）
struct BiquadLP {
    void set(float hz, float q, double sr) noexcept {
        float w = 2.0f * 3.14159265f * hz / (float) sr;
        if (w > 3.0f) w = 3.0f;
        float alpha = std::sin(w) / (2.0f * q);
        float cw = std::cos(w);
        float a0 = 1.0f + alpha;
        b0 = (1.0f - cw) * 0.5f / a0;
        b1 = (1.0f - cw) / a0;
        b2 = b0;
        a1 = -2.0f * cw / a0;
        a2 = (1.0f - alpha) / a0;
    }
    float process(float x) noexcept {
        float y = b0 * x + b1 * x1 + b2 * x2 - a1 * y1 - a2 * y2;
        x2 = x1; x1 = x;
        y2 = y1; y1 = y;
        return y;
    }
    float b0 = 1, b1 = 0, b2 = 0, a1 = 0, a2 = 0;
    float x1 = 0, x2 = 0, y1 = 0, y2 = 0;
};

// ゆっくり揺れるランダム値 [0,1]。一定間隔で目標値を引き直し、1-poleで滑らかに追従。
struct SlowRandom {
    void setRate(float hz, double sr, Rng& rng) noexcept {
        interval = (int) ((float) sr / hz);
        smooth.setCutoff(hz * 2.0f, sr);
        target = rng.uniform();
    }
    float next(Rng& rng) noexcept {
        if (++counter >= interval) {
            counter = 0;
            target = rng.uniform();
        }
        return smooth.process(target);
    }
    int interval = 48000, counter = 0;
    float target = 0.5f;
    OnePoleLP smooth;
};

// ポアソン過程トリガー（平均rate回/秒）
struct PoissonTrigger {
    void setRate(float perSecond, double sr) noexcept {
        probability = perSecond / (float) sr;
    }
    bool fire(Rng& rng) noexcept { return rng.uniform() < probability; }
    float probability = 0.0f;
};

} // namespace raiu::tex
