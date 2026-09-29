#include <catch2/catch_test_macros.hpp>
#include "dsp/texture/WindGenerator.h"
#include "dsp/texture/WaterGenerator.h"
#include "dsp/texture/BirdsGenerator.h"
#include "dsp/texture/InsectsGenerator.h"
#include "dsp/texture/RainGenerator.h"
#include <cmath>
#include <vector>

namespace {

constexpr double kSr = 48000.0;
constexpr int kSeconds = 5;

// 全テクスチャジェネレーター共通の性質チェック:
//  1) NaN/inf無し  2) |x|<=1  3) RMSが実用範囲  4) 長期的な「揺らぎ」がある
template <typename Gen>
void checkTextureProperties(Gen& gen) {
    gen.setSampleRate(kSr);
    const int n = (int) kSr * kSeconds;
    std::vector<float> buf((size_t) n);
    for (auto& s : buf) s = gen.next();

    double acc = 0;
    for (auto s : buf) {
        REQUIRE(std::isfinite(s));
        REQUIRE(std::abs(s) <= 1.0f);
        acc += (double) s * s;
    }
    double rms = std::sqrt(acc / n);
    REQUIRE(rms > 0.01);
    REQUIRE(rms < 0.5);

    // 1秒ごとのRMSに変動があること（静的なノイズでない）
    std::vector<double> secRms;
    for (int sec = 0; sec < kSeconds; ++sec) {
        double a = 0;
        for (int i = 0; i < (int) kSr; ++i) {
            float s = buf[(size_t) (sec * (int) kSr + i)];
            a += (double) s * s;
        }
        secRms.push_back(std::sqrt(a / kSr));
    }
    double mean = 0;
    for (auto r : secRms) mean += r;
    mean /= (double) secRms.size();
    double var = 0;
    for (auto r : secRms) var += (r - mean) * (r - mean);
    var /= (double) secRms.size();
    REQUIRE(var > 1e-8);
}

} // namespace

TEST_CASE("WindGenerator has valid texture properties") {
    raiu::WindGenerator gen;
    checkTextureProperties(gen);
}

TEST_CASE("WaterGenerator has valid texture properties") {
    raiu::WaterGenerator gen;
    checkTextureProperties(gen);
}

TEST_CASE("BirdsGenerator has valid texture properties") {
    raiu::BirdsGenerator gen;
    checkTextureProperties(gen);
}

TEST_CASE("InsectsGenerator has valid texture properties") {
    raiu::InsectsGenerator gen;
    checkTextureProperties(gen);
}

TEST_CASE("RainGenerator has valid texture properties") {
    raiu::RainGenerator gen;
    checkTextureProperties(gen);
}

TEST_CASE("RainGenerator density changes intensity") {
    raiu::RainGenerator light, heavy;
    light.setSampleRate(kSr);
    heavy.setSampleRate(kSr);
    light.setDensity(0.15f);
    heavy.setDensity(1.0f);
    double accL = 0, accH = 0;
    for (int i = 0; i < (int) kSr * 2; ++i) {
        float l = light.next(), h = heavy.next();
        accL += (double) l * l;
        accH += (double) h * h;
    }
    REQUIRE(accH > accL * 1.5);
}
