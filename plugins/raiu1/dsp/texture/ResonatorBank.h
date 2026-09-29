#pragma once

#include "TextureCommon.h"
#include <array>
#include <cmath>

namespace raiu {

// 押鍵中の音程に同調した共鳴バンドパス群。テクスチャを通すと
// 環境音が「弾いた和音の色」で鳴る。ノートオフ後はゆっくりフェードアウト。
class ResonatorBank {
public:
    void setSampleRate(double sr) noexcept {
        sampleRate = sr;
        // ノートオフ後 約0.5秒でフェードアウト
        fadeCoef = 1.0f - std::exp(-3.0f / (0.5f * (float) sr));
        for (auto& s : slots) s.updateFilter(sampleRate, q);
    }

    void setResonance(float newQ) noexcept {
        q = newQ < 1.0f ? 1.0f : newQ;
        for (auto& s : slots)
            if (s.note >= 0) s.updateFilter(sampleRate, q);
    }

    void noteOn(int midiNote) noexcept {
        // 既存スロットの再利用 → 空き → 最小ゲインの奪取
        Slot* target = nullptr;
        for (auto& s : slots)
            if (s.note == midiNote) { target = &s; break; }
        if (!target)
            for (auto& s : slots)
                if (s.note < 0) { target = &s; break; }
        if (!target) {
            target = &slots[0];
            for (auto& s : slots)
                if (s.gain < target->gain) target = &s;
        }
        target->note = midiNote;
        target->held = true;
        target->targetGain = 1.0f;
        target->updateFilter(sampleRate, q);
    }

    void noteOff(int midiNote) noexcept {
        for (auto& s : slots)
            if (s.note == midiNote && s.held) {
                s.held = false;
                s.targetGain = 0.0f;
            }
    }

    void allNotesOff() noexcept {
        for (auto& s : slots)
            if (s.note >= 0) { s.held = false; s.targetGain = 0.0f; }
    }

    float process(float in) noexcept {
        float out = 0.0f;
        for (auto& s : slots) {
            if (s.note < 0) continue;
            s.gain += fadeCoef * (s.targetGain - s.gain);
            if (!s.held && s.gain < 1e-4f) { s.note = -1; s.gain = 0.0f; continue; }
            out += s.filter.process(in) * s.gain;
        }
        return out;
    }

private:
    struct Slot {
        int note = -1;
        bool held = false;
        float gain = 0.0f, targetGain = 0.0f;
        tex::BiquadBP filter;

        void updateFilter(double sr, float q) noexcept {
            if (note < 0) return;
            float hz = 440.0f * std::pow(2.0f, (float) (note - 69) / 12.0f);
            // ナイキスト近くは不安定になるため上限を設ける
            float maxHz = (float) sr * 0.45f;
            filter.set(hz > maxHz ? maxHz : hz, q, sr);
        }
    };

    double sampleRate = 48000.0;
    float q = 60.0f;
    float fadeCoef = 0.001f;
    std::array<Slot, 8> slots;
};

} // namespace raiu
