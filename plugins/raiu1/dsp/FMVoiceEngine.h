#pragma once

#include "FMAlgorithm.h"
#include "texture/NatureOscillator.h"
#include <array>
#include <cmath>

namespace raiu {

// 1ボイス分のFMエンジン。ピッチ変換・ベロシティ感度・キースケーリング・
// ポルタメント（1-poleグライド）を統合する。
class FMVoiceEngine {
public:
    struct OpParams {
        float ratio = 1.0f;
        float detuneCents = 0.0f;
        bool fixed = false;
        float fixedHz = 440.0f;
        float level = 1.0f;
        float attack = 0.005f, decay = 0.1f, sustain = 0.8f, release = 0.3f;
        float velSens = 0.0f;           // 0..1
        float keyScaleDbPerOct = 0.0f;  // C4より上で1octごとに減衰するdB量
    };

    struct Params {
        std::array<OpParams, 4> ops;
        int algorithm = 3;
        float feedback = 0.0f;
    };

    void setSampleRate(double sr) noexcept {
        sampleRate = sr;
        alg.setSampleRate(sr);
        nature.setSampleRate(sr);
        updateGlideCoef();
    }

    void setNatureParams(const NatureOscillator::Params& p) noexcept { nature.setParams(p); }

    void setParams(const Params& p) noexcept {
        params = p;
        alg.setAlgorithm(p.algorithm);
        alg.setFeedback(p.feedback);
        for (int i = 0; i < 4; ++i) {
            auto& op = alg.op(i);
            const auto& q = p.ops[(size_t) i];
            if (q.fixed) op.setFixedFrequency(q.fixedHz);
            else op.setRatio(q.ratio);
            op.setDetune(q.detuneCents);
            op.setEnvParams(q.attack, q.decay, q.sustain, q.release);
        }
        applyLevels();
    }

    void setPortamentoTime(float seconds) noexcept {
        portamentoTime = seconds;
        updateGlideCoef();
    }

    void noteOn(int midiNote, float velocity) noexcept {
        note = midiNote;
        vel = velocity;
        targetHz = midiToHz(midiNote);
        if (!isActive() || portamentoTime <= 0.0f)
            currentHz = targetHz;  // 新規発音は即時ピッチ
        applyLevels();
        alg.noteOn(currentHz * pitchFactor);
        nature.noteOn(currentHz * pitchFactor);
    }

    // Breath: モジュレーターopのレベルだけを一時的に増減して音色を「開く」
    void setModulatorBoost(float boost) noexcept {
        if (boost != modBoost) {
            modBoost = boost;
            applyLevels();
        }
    }

    // ビブラート・ピッチベンド用のグローバルなピッチオフセット（セント）
    void setPitchOffsetCents(float cents) noexcept {
        float f = std::pow(2.0f, cents / 1200.0f);
        if (f != pitchFactor) {
            pitchFactor = f;
            pitchDirty = true;
        }
    }

    void noteOff() noexcept {
        alg.noteOff();
        nature.noteOff();
    }
    void reset() noexcept {
        alg.reset();
        nature.reset();
    }
    bool isActive() const noexcept { return alg.isActive() || nature.isActive(); }
    int currentNote() const noexcept { return note; }

    void render(float* out, int n) noexcept {
        const bool gliding = std::abs(targetHz - currentHz) > 0.01f;
        if (pitchDirty && !gliding) {
            alg.setNoteFrequency(currentHz * pitchFactor);
            pitchDirty = false;
        }
        for (int i = 0; i < n; ++i) {
            if (gliding) {
                currentHz += glideCoef * (targetHz - currentHz);
                alg.setNoteFrequency(currentHz * pitchFactor);
            }
            out[i] += alg.renderSample();
        }
        if (gliding) pitchDirty = false;
        nature.render(out, n);
    }

    static float midiToHz(int midiNote) noexcept {
        return 440.0f * std::pow(2.0f, (float) (midiNote - 69) / 12.0f);
    }

private:
    void applyLevels() noexcept {
        for (int i = 0; i < 4; ++i) {
            const auto& q = params.ops[(size_t) i];
            float velFactor = 1.0f - q.velSens * (1.0f - vel);
            float octsAboveC4 = std::max(0.0f, (float) (note - 60) / 12.0f);
            float ksFactor = std::pow(10.0f, -q.keyScaleDbPerOct * octsAboveC4 / 20.0f);
            float boost = alg.isCarrier(i) ? 1.0f : modBoost;
            alg.op(i).setLevel(q.level * velFactor * ksFactor * boost);
        }
    }

    void updateGlideCoef() noexcept {
        glideCoef = portamentoTime <= 0.0f
                        ? 1.0f
                        : 1.0f - std::exp(-3.0f / (portamentoTime * (float) sampleRate));
    }

    Params params;
    FMAlgorithm alg;
    NatureOscillator nature;
    double sampleRate = 48000.0;
    int note = 69;
    float vel = 1.0f;
    float currentHz = 440.0f, targetHz = 440.0f;
    float portamentoTime = 0.0f, glideCoef = 1.0f;
    float pitchFactor = 1.0f;
    bool pitchDirty = false;
    float modBoost = 1.0f;
};

} // namespace raiu
