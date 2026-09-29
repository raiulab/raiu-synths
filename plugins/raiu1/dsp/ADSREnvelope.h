#pragma once

#include <algorithm>
#include <cmath>

namespace raiu {

// 状態機械型ADSR。各セグメントは1-pole指数カーブで目標値へ漸近する。
// アタックは目標をオーバーシュート気味(1.3)に置いて実時間で1.0に到達させる定石を使う。
class ADSREnvelope {
public:
    void setSampleRate(double sr) noexcept {
        sampleRate = sr;
        updateCoefs();
    }

    // a, d, r は秒、s はレベル(0..1)
    void setParams(float a, float d, float s, float r) noexcept {
        attackTime = std::max(0.001f, a);
        decayTime = std::max(0.001f, d);
        sustainLevel = std::clamp(s, 0.0f, 1.0f);
        releaseTime = std::max(0.001f, r);
        updateCoefs();
    }

    void noteOn() noexcept { state = State::Attack; }
    void noteOff() noexcept {
        if (state != State::Idle) state = State::Release;
    }
    void reset() noexcept { state = State::Idle; value = 0.0f; }

    bool isActive() const noexcept { return state != State::Idle; }

    float next() noexcept {
        switch (state) {
            case State::Idle:
                return 0.0f;
            case State::Attack:
                value += attackCoef * (kAttackTarget - value);
                if (value >= 1.0f) { value = 1.0f; state = State::Decay; }
                break;
            case State::Decay:
                value += decayCoef * (sustainLevel - value);
                if (std::abs(value - sustainLevel) < 1e-4f) state = State::Sustain;
                break;
            case State::Sustain:
                value = sustainLevel;
                break;
            case State::Release:
                value += releaseCoef * (0.0f - value);
                if (value < kSilence) { value = 0.0f; state = State::Idle; }
                break;
        }
        return std::clamp(value, 0.0f, 1.0f);
    }

private:
    enum class State { Idle, Attack, Decay, Sustain, Release };

    static constexpr float kAttackTarget = 1.3f;
    static constexpr float kSilence = 1e-4f;

    void updateCoefs() noexcept {
        attackCoef = coefFor(attackTime);
        decayCoef = coefFor(decayTime);
        releaseCoef = coefFor(releaseTime);
    }

    // time秒で目標値の約95%へ到達する1-pole係数（時定数3個分）
    float coefFor(float time) const noexcept {
        return 1.0f - std::exp(-3.0f / (time * (float) sampleRate));
    }

    double sampleRate = 48000.0;
    float attackTime = 0.01f, decayTime = 0.1f, sustainLevel = 1.0f, releaseTime = 0.2f;
    float attackCoef = 0.01f, decayCoef = 0.01f, releaseCoef = 0.01f;
    float value = 0.0f;
    State state = State::Idle;
};

} // namespace raiu
