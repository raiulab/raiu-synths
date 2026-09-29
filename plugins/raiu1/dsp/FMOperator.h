#pragma once

#include "ADSREnvelope.h"
#include "SineTable.h"
#include <algorithm>
#include <cmath>

namespace raiu {

// FMオペレーター1基。位相累積 + 位相変調入力 + 自己フィードバック。
// modInput は位相単位（1.0 = 1周期）。
class FMOperator {
public:
    void setSampleRate(double sr) noexcept {
        sampleRate = sr;
        env.setSampleRate(sr);
        updateIncrement();
    }

    void setRatio(float r) noexcept { ratio = r; fixedMode = false; updateIncrement(); }
    void setDetune(float cents) noexcept {
        detuneFactor = std::pow(2.0f, cents / 1200.0f);
        updateIncrement();
    }
    void setFixedFrequency(float hz) noexcept { fixedHz = hz; fixedMode = true; updateIncrement(); }
    void setLevel(float l) noexcept { level = std::clamp(l, 0.0f, 1.0f); }
    void setFeedback(float f) noexcept { feedback = std::clamp(f, 0.0f, 1.0f); }
    void setEnvParams(float a, float d, float s, float r) noexcept { env.setParams(a, d, s, r); }

    void noteOn(float noteHz) noexcept {
        baseHz = noteHz;
        updateIncrement();
        // レガート再トリガー（モノモードで前の音がまだ鳴っている間の弾き直し）では
        // 位相を0へ飛ばすと波形が不連続になりクリック/クラックルの原因になるため、
        // 発音中でなかった場合のみ位相をリセットする
        if (!env.isActive()) {
            phase = 0.0f;
            prevOut = 0.0f;
        }
        env.noteOn();
    }
    void noteOff() noexcept { env.noteOff(); }
    void reset() noexcept { env.reset(); phase = 0.0f; prevOut = 0.0f; }

    // 発音中にピッチだけ変える（ポルタメント・ベンド用）
    void setNoteFrequency(float noteHz) noexcept { baseHz = noteHz; updateIncrement(); }

    bool isActive() const noexcept { return env.isActive(); }

    float next(float modInput) noexcept {
        // DX式フィードバック: 自己出力を位相へ帰還（最大約半周期）
        float fb = feedback * prevOut * 0.5f;
        float out = level * env.next() * SineTable::lookup(phase + modInput + fb);
        prevOut = out;
        phase += increment;
        phase -= std::floor(phase);
        return out;
    }

private:
    void updateIncrement() noexcept {
        float hz = fixedMode ? fixedHz : baseHz * ratio * detuneFactor;
        increment = (float) ((double) hz / sampleRate);
    }

    double sampleRate = 48000.0;
    float ratio = 1.0f, detuneFactor = 1.0f, fixedHz = 440.0f;
    bool fixedMode = false;
    float baseHz = 440.0f;
    float level = 1.0f, feedback = 0.0f;
    float phase = 0.0f, increment = 0.0f, prevOut = 0.0f;
    ADSREnvelope env;
};

} // namespace raiu
