#pragma once

#include "TextureCommon.h"
#include "../ADSREnvelope.h"
#include "../SineTable.h"
#include <array>

namespace raiu {

// 自然音を「音素」として鍵盤で演奏するボイス内蔵オシレーター。
// KeyTrack=1 で押鍵の音程に完全追従し、専用ADSRとレゾナンスLPで造形して
// FMコアとボイス内ミックスされる。
class NatureOscillator {
public:
    enum class Mode { Off = 0, Wind, Water, Birds, Insects, Rain };

    struct Params {
        Mode mode = Mode::Off;
        float level = 0.5f;
        float keyTrack = 1.0f;     // 0=固定音程, 1=完全追従
        float semitones = 0.0f;    // ピッチオフセット（±24半音）
        float attack = 0.05f, decay = 0.3f, sustain = 0.7f, release = 0.4f;
        float cutoff = 8000.0f;    // 専用LPフィルター
        float resonance = 0.2f;    // 0..1
    };

    void setSampleRate(double sr) noexcept {
        sampleRate = sr;
        env.setSampleRate(sr);
        gust.setRate(0.5f, sr, rng);
        retune();
        updateFilter();
    }

    void setParams(const Params& p) noexcept {
        params = p;
        env.setParams(p.attack, p.decay, p.sustain, p.release);
        retune();
        updateFilter();
    }

    void noteOn(float hz) noexcept {
        if (params.mode == Mode::Off) return;
        noteHz = hz;
        retune();
        env.noteOn();
    }
    void noteOff() noexcept { env.noteOff(); }
    void reset() noexcept { env.reset(); }
    bool isActive() const noexcept { return env.isActive(); }

    // 出力はバッファへ加算
    void render(float* out, int n) noexcept {
        if (params.mode == Mode::Off || !env.isActive()) return;
        for (int i = 0; i < n; ++i) {
            float s = generate();
            out[i] += filter.process(s) * env.next() * params.level;
        }
    }

private:
    // モードごとの基準周波数（KeyTrack=0のときの音程）
    static float baseHzFor(Mode m) noexcept {
        switch (m) {
            case Mode::Wind: return 440.0f;
            case Mode::Water: return 700.0f;
            case Mode::Birds: return 3000.0f;
            case Mode::Insects: return 4400.0f;
            case Mode::Rain: return 1800.0f;
            default: return 440.0f;
        }
    }

    void retune() noexcept {
        float base = baseHzFor(params.mode);
        float target = noteHz * std::pow(2.0f, params.semitones / 12.0f);
        // 幾何補間: kt=0で基準周波数、kt=1で音程完全追従
        effHz = base * std::pow(target / base, params.keyTrack);
        effHz = std::clamp(effHz, 20.0f, (float) sampleRate * 0.4f);

        windBand1.set(effHz, 30.0f, sampleRate);
        windBand2.set(std::min(effHz * 2.0f, (float) sampleRate * 0.45f), 20.0f, sampleRate);
        rainBand.set(effHz, 40.0f, sampleRate);
        bubbleTrigger.setRate(12.0f, sampleRate);
        dropTrigger.setRate(25.0f, sampleRate);
    }

    void updateFilter() noexcept {
        float q = 0.707f + params.resonance * 7.0f;
        filter.set(params.cutoff, q, sampleRate);
    }

    float generate() noexcept {
        switch (params.mode) {
            case Mode::Wind: {
                // 風笛: 音程に同調した高Qバンドパスのノイズ + 突風の揺れ
                float noise = rng.bipolar();
                float g = 0.5f + 0.5f * gust.next(rng);
                return (windBand1.process(noise) * 4.0f
                        + windBand2.process(noise) * 1.2f) * g;
            }
            case Mode::Water: {
                // 音程付きの泡: effHzから上昇するチャープ
                if (bubbleTrigger.fire(rng)) spawnBubble();
                float s = 0.0f;
                for (auto& b : bubbles) {
                    if (b.amp <= 1e-4f) continue;
                    b.phase += b.hz / (float) sampleRate;
                    b.phase -= std::floor(b.phase);
                    s += b.amp * SineTable::lookup(b.phase);
                    b.hz *= b.rise;
                    b.amp *= b.decay;
                }
                return s * 0.7f;
            }
            case Mode::Birds: {
                // 音階のさえずり: effHzを中心に弧を描くチャープの繰り返し
                if (chirpRemaining <= 0) {
                    if (--chirpGap <= 0) spawnChirp();
                    return 0.0f;
                }
                float t = 1.0f - (float) chirpRemaining / (float) chirpDuration;
                float u = 1.0f - t;
                float hz = chirpF0 * u * u + 2.0f * chirpFPeak * u * t + chirpF1 * t * t;
                chirpPhase += hz / (float) sampleRate;
                chirpPhase -= std::floor(chirpPhase);
                // 0→1→0の山型窓（以前は単調増加のまま打ち切られ、さえずり終わりに
                // 毎回振幅最大からの瞬断クリックが出ていた）
                float window = SineTable::lookup(0.5f * t);
                --chirpRemaining;
                return 0.6f * window * window * SineTable::lookup(chirpPhase);
            }
            case Mode::Insects: {
                // 音程キャリアの虫: effHzの正弦をパルストレインでAM
                pulsePhase += 28.0f / (float) sampleRate;
                pulsePhase -= std::floor(pulsePhase);
                float p = std::max(0.0f, SineTable::lookup(pulsePhase));
                p = p * p * p * p;
                carrierPhase += effHz / (float) sampleRate;
                carrierPhase -= std::floor(carrierPhase);
                return 0.7f * p * SineTable::lookup(carrierPhase);
            }
            case Mode::Rain: {
                // 調律された雨粒: インパルスを音程の共鳴に通す
                float excitation = 0.0f;
                if (dropTrigger.fire(rng)) {
                    float a = rng.uniform();
                    excitation = rng.bipolar() * (1.0f + 3.0f * a * a);
                }
                return rainBand.process(excitation) * 2.5f;
            }
            default:
                return 0.0f;
        }
    }

    void spawnBubble() noexcept {
        for (auto& b : bubbles) {
            if (b.amp > 1e-4f) continue;
            b.phase = 0;
            b.hz = effHz * (0.8f + 0.15f * rng.uniform());
            b.amp = 0.3f + 0.3f * rng.uniform();
            float lifeSec = 0.06f + 0.05f * rng.uniform();
            b.decay = std::exp(-1.0f / (lifeSec * 0.3f * (float) sampleRate));
            b.rise = std::exp(0.5f / (lifeSec * (float) sampleRate));
            return;
        }
    }

    void spawnChirp() noexcept {
        chirpF0 = effHz * (0.85f + 0.1f * rng.uniform());
        chirpFPeak = effHz * (1.15f + 0.2f * rng.uniform());
        chirpF1 = effHz * (0.95f + 0.1f * rng.uniform());
        chirpDuration = (int) ((0.08f + 0.1f * rng.uniform()) * (float) sampleRate);
        chirpRemaining = chirpDuration;
        chirpPhase = 0.0f;
        chirpGap = (int) ((0.08f + 0.25f * rng.uniform()) * (float) sampleRate);
    }

    struct Bubble {
        float phase = 0, hz = 0, amp = 0, rise = 1, decay = 1;
    };

    double sampleRate = 48000.0;
    Params params;
    ADSREnvelope env;
    tex::Rng rng{0xABCD1234u};
    tex::BiquadLP filter;
    float noteHz = 440.0f, effHz = 440.0f;

    tex::BiquadBP windBand1, windBand2, rainBand;
    tex::SlowRandom gust;
    tex::PoissonTrigger bubbleTrigger, dropTrigger;
    std::array<Bubble, 4> bubbles;
    float carrierPhase = 0, pulsePhase = 0;
    float chirpF0 = 0, chirpFPeak = 0, chirpF1 = 0, chirpPhase = 0;
    int chirpDuration = 1, chirpRemaining = 0, chirpGap = 0;
};

} // namespace raiu
