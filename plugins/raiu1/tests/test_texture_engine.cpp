#include <catch2/catch_test_macros.hpp>
#include "dsp/texture/TextureEngine.h"
#include <cmath>
#include <vector>

namespace {
constexpr double kSr = 48000.0;

float renderRms(raiu::TextureEngine& e, int n) {
    std::vector<float> buf((size_t) n, 0.0f);
    e.process(buf.data(), n);
    double acc = 0;
    for (auto s : buf) {
        REQUIRE(std::isfinite(s));
        acc += (double) s * s;
    }
    return (float) std::sqrt(acc / n);
}
} // namespace

TEST_CASE("TextureEngine is silent when all levels are zero") {
    raiu::TextureEngine e;
    e.setSampleRate(kSr);
    raiu::TextureEngine::Params p;  // 既定は全レベル0
    e.setParams(p);
    REQUIRE(renderRms(e, (int) kSr) < 1e-6f);
}

TEST_CASE("TextureEngine outputs wind when wind level is up") {
    raiu::TextureEngine e;
    e.setSampleRate(kSr);
    raiu::TextureEngine::Params p;
    p.windLevel = 0.8f;
    p.resonanceMix = 0.0f;  // 素のテクスチャのみ
    e.setParams(p);
    REQUIRE(renderRms(e, (int) kSr * 2) > 0.005f);
}

TEST_CASE("TextureEngine resonanceMix=1 with no notes is silent") {
    raiu::TextureEngine e;
    e.setSampleRate(kSr);
    raiu::TextureEngine::Params p;
    p.windLevel = 0.8f;
    p.resonanceMix = 1.0f;  // 共鳴のみ・押鍵なし → ほぼ無音
    e.setParams(p);
    renderRms(e, (int) kSr);  // フィルター整定
    REQUIRE(renderRms(e, (int) kSr) < 0.01f);

    e.noteOn(69);  // 押鍵すると共鳴が鳴り出す
    renderRms(e, 4800);
    REQUIRE(renderRms(e, (int) kSr) > 0.005f);
}

TEST_CASE("TextureEngine distance darkens the sound") {
    raiu::TextureEngine near, far;
    near.setSampleRate(kSr);
    far.setSampleRate(kSr);
    raiu::TextureEngine::Params p;
    p.rainLevel = 0.8f;
    p.resonanceMix = 0.0f;
    p.distance = 0.0f;
    near.setParams(p);
    p.distance = 1.0f;
    far.setParams(p);
    float rNear = renderRms(near, (int) kSr * 2);
    float rFar = renderRms(far, (int) kSr * 2);
    REQUIRE(rFar < rNear * 0.7f);  // 高域が削られてエネルギーが落ちる
}
