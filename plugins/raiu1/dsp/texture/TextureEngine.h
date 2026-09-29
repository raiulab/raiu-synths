#pragma once

#include "WindGenerator.h"
#include "WaterGenerator.h"
#include "BirdsGenerator.h"
#include "InsectsGenerator.h"
#include "RainGenerator.h"
#include "ResonatorBank.h"

namespace raiu {

// 5つの環境音ジェネレーター + キートラッキング共鳴のミックスバス。
// 出力は加算（+=）でバッファに書く。
class TextureEngine {
public:
    struct Params {
        float windLevel = 0.0f;
        float waterLevel = 0.0f;
        float birdsLevel = 0.0f;
        float insectsLevel = 0.0f;
        float rainLevel = 0.0f;
        float rainDensity = 0.5f;
        float resonance = 60.0f;     // 共鳴Q
        float resonanceMix = 0.7f;   // 0=素のテクスチャ, 1=共鳴のみ
        float distance = 0.3f;       // 0=近い, 1=遠い（LPで暗くなる）
        float master = 0.8f;
    };

    void setSampleRate(double sr) noexcept {
        sampleRate = sr;
        wind.setSampleRate(sr);
        water.setSampleRate(sr);
        birds.setSampleRate(sr);
        insects.setSampleRate(sr);
        rain.setSampleRate(sr);
        resonator.setSampleRate(sr);
        applyDistance();
    }

    void setParams(const Params& p) noexcept {
        params = p;
        rain.setDensity(p.rainDensity);
        resonator.setResonance(p.resonance);
        // 狭帯域ほど通過エネルギーが減るため√Qでゲイン補償する（ポリフォニー時の
        // 過大な過渡スパイクを避けるため上限を設ける）
        resonatorGain = std::min(4.5f, 1.2f * std::sqrt(p.resonance));
        applyDistance();
    }

    void noteOn(int midiNote) noexcept { resonator.noteOn(midiNote); }
    void noteOff(int midiNote) noexcept { resonator.noteOff(midiNote); }
    void allNotesOff() noexcept { resonator.allNotesOff(); }

    // 直近ブロックの素テクスチャRMS（Breath変調のエンベロープ源として使う）
    float lastRawRms() const noexcept { return rawRms; }

    void process(float* buf, int n) noexcept {
        if (n <= 0) return;
        const auto& p = params;
        const bool anyLevel = p.windLevel > 1e-5f || p.waterLevel > 1e-5f
                              || p.birdsLevel > 1e-5f || p.insectsLevel > 1e-5f
                              || p.rainLevel > 1e-5f;
        if (!anyLevel) {
            rawRms = 0.0f;
            return;
        }
        double acc = 0.0;
        for (int i = 0; i < n; ++i) {
            float raw = 0.0f;
            if (p.windLevel > 1e-5f) raw += wind.next() * p.windLevel;
            if (p.waterLevel > 1e-5f) raw += water.next() * p.waterLevel;
            if (p.birdsLevel > 1e-5f) raw += birds.next() * p.birdsLevel;
            if (p.insectsLevel > 1e-5f) raw += insects.next() * p.insectsLevel;
            if (p.rainLevel > 1e-5f) raw += rain.next() * p.rainLevel;
            acc += (double) raw * raw;

            float res = resonator.process(raw) * resonatorGain;
            float mixed = raw + p.resonanceMix * (res - raw);
            // 高Q+ポリフォニーで生じ得る過渡スパイクの安全弁（通常レベルでは実質無色）
            mixed = std::tanh(mixed * 0.8f) * 1.25f;
            buf[i] += distanceLP.process(mixed) * p.master;
        }
        rawRms = (float) std::sqrt(acc / n);
    }

private:
    void applyDistance() noexcept {
        // 近い=9kHz 〜 遠い=700Hz
        float cutoff = 9000.0f * std::pow(700.0f / 9000.0f, params.distance);
        distanceLP.setCutoff(cutoff, sampleRate);
    }

    double sampleRate = 48000.0;
    Params params;
    WindGenerator wind;
    WaterGenerator water;
    BirdsGenerator birds;
    InsectsGenerator insects;
    RainGenerator rain;
    ResonatorBank resonator;
    tex::OnePoleLP distanceLP;
    float resonatorGain = 9.3f;
    float rawRms = 0.0f;
};

} // namespace raiu
