#pragma once

#include <array>
#include <cmath>

namespace raiu {

// 正弦波ルックアップテーブル。位相は [0,1) = 1周期。範囲外は折り返す。
class SineTable {
public:
    static float lookup(float phase) noexcept {
        const auto& t = table();
        float p = phase - std::floor(phase);
        float idx = p * (float) kSize;
        int i0 = (int) idx;
        float frac = idx - (float) i0;
        int i1 = (i0 + 1) & (kSize - 1);
        return t[(size_t) (i0 & (kSize - 1))] * (1.0f - frac) + t[(size_t) i1] * frac;
    }

private:
    static constexpr int kSize = 4096;

    static const std::array<float, kSize>& table() noexcept {
        static const std::array<float, kSize> t = [] {
            std::array<float, kSize> a{};
            for (int i = 0; i < kSize; ++i)
                a[(size_t) i] = (float) std::sin(2.0 * M_PI * (double) i / (double) kSize);
            return a;
        }();
        return t;
    }
};

} // namespace raiu
