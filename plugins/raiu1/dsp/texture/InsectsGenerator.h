#pragma once

#include "TextureCommon.h"
#include "../SineTable.h"
#include <array>

namespace raiu {

// 虫: 高域正弦キャリアをパルストレインでAM変調（鈴虫・コオロギ）。
// 「鳴いて休む」周期パターンの個体を複数重ねる。
class InsectsGenerator {
public:
    void setSampleRate(double sr) noexcept {
        sampleRate = sr;
        for (size_t i = 0; i < voices.size(); ++i) {
            auto& v = voices[i];
            v.carrierHz = 3800.0f + 900.0f * rng.uniform();
            v.pulseHz = 24.0f + 14.0f * rng.uniform();
            v.songHz = 0.35f + 0.5f * rng.uniform();   // 鳴き〜休みの周期
            v.songPhase = rng.uniform();
            v.amp = 0.28f + 0.12f * rng.uniform();
        }
    }

    float next() noexcept {
        float out = 0.0f;
        for (auto& v : voices) {
            // 鳴きパターン: songPhaseの前半だけ鳴く（デューティ約55%）
            v.songPhase += v.songHz / (float) sampleRate;
            v.songPhase -= std::floor(v.songPhase);
            if (v.songPhase > 0.55f) continue;

            // パルストレイン: 正弦を尖らせて「リッリッリッ」を作る
            v.pulsePhase += v.pulseHz / (float) sampleRate;
            v.pulsePhase -= std::floor(v.pulsePhase);
            float p = std::max(0.0f, SineTable::lookup(v.pulsePhase));
            p = p * p * p * p;

            v.carrierPhase += v.carrierHz / (float) sampleRate;
            v.carrierPhase -= std::floor(v.carrierPhase);
            out += v.amp * p * SineTable::lookup(v.carrierPhase);
        }
        return out;
    }

private:
    struct Voice {
        float carrierHz = 4200, pulseHz = 28, songHz = 0.5f, amp = 0.3f;
        float carrierPhase = 0, pulsePhase = 0, songPhase = 0;
    };

    double sampleRate = 48000.0;
    tex::Rng rng{777u};
    std::array<Voice, 3> voices;
};

} // namespace raiu
