#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>
#include "dsp/FMAlgorithm.h"
#include <cmath>
#include <vector>

namespace {

constexpr double kSr = 48000.0;

// 全op同一設定のスタックを作る
raiu::FMAlgorithm makeStack(int algorithm) {
    raiu::FMAlgorithm alg;
    alg.setSampleRate(kSr);
    alg.setAlgorithm(algorithm);
    for (int i = 0; i < 4; ++i) {
        alg.op(i).setRatio(1.0f);
        alg.op(i).setLevel(1.0f);
        alg.op(i).setEnvParams(0.001f, 0.1f, 1.0f, 0.1f);
    }
    return alg;
}

float rms(raiu::FMAlgorithm& alg, int n) {
    double acc = 0;
    for (int i = 0; i < n; ++i) {
        float s = alg.renderSample();
        acc += (double) s * s;
    }
    return (float) std::sqrt(acc / n);
}

} // namespace

TEST_CASE("Algorithm 8 sums all four carriers") {
    auto alg = makeStack(8);
    alg.noteOn(440.0f);
    auto one = makeStack(8);
    for (int i = 1; i < 4; ++i) one.op(i).setLevel(0.0f);
    one.noteOn(440.0f);
    // 全opが同位相・同周波数なので出力は単一opの4倍
    float r4 = rms(alg, 4800), r1 = rms(one, 4800);
    REQUIRE(r4 == Catch::Approx(4.0f * r1).epsilon(0.01f));
}

TEST_CASE("Algorithm 1 outputs only op1 (serial chain)") {
    auto alg = makeStack(1);
    alg.op(0).setLevel(0.0f);  // op1のみキャリア → level 0で完全無音
    alg.noteOn(440.0f);
    REQUIRE(rms(alg, 4800) < 1e-6f);
}

TEST_CASE("Algorithm 3 has two independent pairs") {
    // (2→1)+(4→3): キャリアはop1とop3
    auto silenced = makeStack(3);
    silenced.op(0).setLevel(0.0f);
    silenced.op(2).setLevel(0.0f);
    silenced.noteOn(440.0f);
    REQUIRE(rms(silenced, 4800) < 1e-6f);

    auto carriersOnly = makeStack(3);
    carriersOnly.op(1).setLevel(0.0f);
    carriersOnly.op(3).setLevel(0.0f);
    carriersOnly.noteOn(440.0f);
    REQUIRE(rms(carriersOnly, 4800) > 0.1f);  // キャリアは生きている
}

TEST_CASE("Modulator level shapes the timbre (algorithm 1)") {
    auto plain = makeStack(1);
    for (int i = 1; i < 4; ++i) plain.op(i).setLevel(0.0f);  // 変調なし=純音
    plain.noteOn(440.0f);
    auto modulated = makeStack(1);
    modulated.noteOn(440.0f);
    bool differs = false;
    for (int i = 0; i < 4800; ++i)
        if (std::abs(plain.renderSample() - modulated.renderSample()) > 1e-2f) differs = true;
    REQUIRE(differs);
}

TEST_CASE("All 8 algorithms produce finite bounded output") {
    for (int a = 1; a <= 8; ++a) {
        auto alg = makeStack(a);
        alg.setFeedback(1.0f);
        alg.noteOn(880.0f);
        for (int i = 0; i < 24000; ++i) {
            float s = alg.renderSample();
            REQUIRE(std::isfinite(s));
            REQUIRE(std::abs(s) <= 4.0f);
        }
    }
}

TEST_CASE("noteOff leads to silence and inactive") {
    auto alg = makeStack(3);
    alg.noteOn(440.0f);
    for (int i = 0; i < 4800; ++i) alg.renderSample();
    alg.noteOff();
    for (int i = 0; i < 48000; ++i) alg.renderSample();
    REQUIRE_FALSE(alg.isActive());
    REQUIRE(std::abs(alg.renderSample()) < 1e-4f);
}
