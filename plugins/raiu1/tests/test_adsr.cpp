#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>
#include "dsp/ADSREnvelope.h"
#include <cmath>

namespace {
raiu::ADSREnvelope makeEnv(float a, float d, float s, float r) {
    raiu::ADSREnvelope env;
    env.setSampleRate(48000.0);
    env.setParams(a, d, s, r);
    return env;
}
} // namespace

TEST_CASE("ADSR reaches ~1.0 at end of attack") {
    auto env = makeEnv(0.01f, 0.1f, 0.5f, 0.2f);
    env.noteOn();
    float peak = 0;
    for (int i = 0; i < 480; ++i) peak = std::max(peak, env.next());
    REQUIRE(peak > 0.99f);
}

TEST_CASE("ADSR decays to sustain level") {
    auto env = makeEnv(0.01f, 0.05f, 0.5f, 0.2f);
    env.noteOn();
    float v = 0;
    // アタック + ディケイ×5 経過後はサスティンレベル近傍
    for (int i = 0; i < 480 + 12000; ++i) v = env.next();
    REQUIRE(v == Catch::Approx(0.5f).margin(0.05f));
}

TEST_CASE("ADSR releases below 0.001 and reports inactive") {
    auto env = makeEnv(0.005f, 0.05f, 0.7f, 0.1f);
    env.noteOn();
    for (int i = 0; i < 24000; ++i) env.next();
    env.noteOff();
    float v = 1;
    for (int i = 0; i < 48000; ++i) v = env.next();  // release×10
    REQUIRE(v < 0.001f);
    REQUIRE_FALSE(env.isActive());
}

TEST_CASE("ADSR output is always in [0,1] and finite") {
    auto env = makeEnv(0.02f, 0.1f, 0.3f, 0.15f);
    env.noteOn();
    for (int i = 0; i < 24000; ++i) {
        float v = env.next();
        REQUIRE(std::isfinite(v));
        REQUIRE(v >= 0.0f);
        REQUIRE(v <= 1.0f);
    }
    env.noteOff();
    for (int i = 0; i < 24000; ++i) {
        float v = env.next();
        REQUIRE(std::isfinite(v));
        REQUIRE(v >= 0.0f);
        REQUIRE(v <= 1.0f);
    }
}

TEST_CASE("ADSR is inactive before noteOn and outputs zero") {
    auto env = makeEnv(0.01f, 0.1f, 0.5f, 0.2f);
    REQUIRE_FALSE(env.isActive());
    REQUIRE(env.next() == 0.0f);
}
