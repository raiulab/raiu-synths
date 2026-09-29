#pragma once

#include <cmath>

namespace raiu {

// エンベロープフォロワー: 整流 → attack/release別の1-pole平滑。
// テクスチャの揺らぎをFM音色の「呼吸」に変換するために使う。
class EnvFollower {
public:
    void setSampleRate(double sr, float attackSec = 0.01f, float releaseSec = 0.2f) noexcept {
        attackCoef = 1.0f - std::exp(-1.0f / (attackSec * (float) sr));
        releaseCoef = 1.0f - std::exp(-1.0f / (releaseSec * (float) sr));
    }

    float process(float x) noexcept {
        float level = std::abs(x);
        float coef = level > value ? attackCoef : releaseCoef;
        value += coef * (level - value);
        return value;
    }

    float current() const noexcept { return value; }

private:
    float attackCoef = 0.01f, releaseCoef = 0.001f, value = 0.0f;
};

} // namespace raiu
