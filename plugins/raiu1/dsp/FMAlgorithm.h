#pragma once

#include "FMOperator.h"
#include <array>
#include <cstdint>

namespace raiu {

// 4オペレーターFMのアルゴリズム（ルーティング）8種。
// 変調はすべて高番号op→低番号opへ流れるため、op4から降順にレンダリングすれば
// 同一サンプル内の変調が成立する。フィードバックは常にop4（index 3）。
//
//  # | 構成             | キャリア
//  1 | 4→3→2→1          | 1
//  2 | (3+4)→2→1        | 1
//  3 | (2→1)+(4→3)      | 1,3
//  4 | (2+3+4)→1        | 1
//  5 | (2→1)+3+4        | 1,3,4
//  6 | 4→(1,2,3)        | 1,2,3
//  7 | (4→3→2)+1        | 1,2
//  8 | 1+2+3+4          | 1,2,3,4
class FMAlgorithm {
public:
    void setSampleRate(double sr) noexcept {
        for (auto& o : ops) o.setSampleRate(sr);
    }

    // algorithm: 1..8
    void setAlgorithm(int algorithm) noexcept {
        algoIndex = std::clamp(algorithm, 1, 8) - 1;
    }
    int getAlgorithm() const noexcept { return algoIndex + 1; }

    FMOperator& op(int index) noexcept { return ops[(size_t) index]; }
    const FMOperator& op(int index) const noexcept { return ops[(size_t) index]; }

    void setFeedback(float f) noexcept { ops[3].setFeedback(f); }

    void noteOn(float noteHz) noexcept {
        for (auto& o : ops) o.noteOn(noteHz);
    }
    void noteOff() noexcept {
        for (auto& o : ops) o.noteOff();
    }
    void reset() noexcept {
        for (auto& o : ops) o.reset();
    }
    void setNoteFrequency(float noteHz) noexcept {
        for (auto& o : ops) o.setNoteFrequency(noteHz);
    }

    bool isCarrier(int opIndex) const noexcept {
        return (kSpecs[(size_t) algoIndex].carriers & (1u << opIndex)) != 0;
    }

    bool isActive() const noexcept {
        for (const auto& o : ops)
            if (o.isActive()) return true;
        return false;
    }

    float renderSample() noexcept {
        const auto& spec = kSpecs[(size_t) algoIndex];
        std::array<float, 4> out{};
        float mix = 0.0f;
        for (int i = 3; i >= 0; --i) {
            float mod = 0.0f;
            uint8_t sources = spec.modSources[(size_t) i];
            for (int j = i + 1; j < 4; ++j)
                if (sources & (1u << j)) mod += out[(size_t) j];
            out[(size_t) i] = ops[(size_t) i].next(mod * kModScale);
            if (spec.carriers & (1u << i)) mix += out[(size_t) i];
        }
        return mix;
    }

private:
    struct Spec {
        std::array<uint8_t, 4> modSources;  // op i を変調するopのビットマスク
        uint8_t carriers;                   // キャリアのビットマスク
    };

    // ビットはop index（0=op1 .. 3=op4）
    static constexpr std::array<Spec, 8> kSpecs{{
        {{0b0010, 0b0100, 0b1000, 0}, 0b0001},          // 1: 4→3→2→1
        {{0b0010, 0b1100, 0, 0}, 0b0001},               // 2: (3+4)→2→1
        {{0b0010, 0, 0b1000, 0}, 0b0101},               // 3: (2→1)+(4→3)
        {{0b1110, 0, 0, 0}, 0b0001},                    // 4: (2+3+4)→1
        {{0b0010, 0, 0, 0}, 0b1101},                    // 5: (2→1)+3+4
        {{0b1000, 0b1000, 0b1000, 0}, 0b0111},          // 6: 4→(1,2,3)
        {{0, 0b0100, 0b1000, 0}, 0b0011},               // 7: (4→3→2)+1
        {{0, 0, 0, 0}, 0b1111},                         // 8: 全並列
    }};

    // 変調の深さスケール（モジュレーターlevel=1で位相±0.4周期 ≒ 実用的なDX級の深さ）
    static constexpr float kModScale = 0.4f;

    std::array<FMOperator, 4> ops;
    int algoIndex = 2;  // 既定はアルゴリズム3（エレピ）
};

} // namespace raiu
