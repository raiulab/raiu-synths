#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>
#include "dsp/EnvFollower.h"
#include <cmath>

namespace {
constexpr double kSr = 48000.0;
}

TEST_CASE("EnvFollower tracks a step input within attack time window") {
    raiu::EnvFollower f;
    f.setSampleRate(kSr);  // attack 10ms / release 200ms
    float v = 0;
    for (int i = 0; i < (int) (0.05 * kSr); ++i) v = f.process(0.8f);
    REQUIRE(v == Catch::Approx(0.8f).margin(0.02f));
}

TEST_CASE("EnvFollower releases slowly after signal stops") {
    raiu::EnvFollower f;
    f.setSampleRate(kSr);
    for (int i = 0; i < (int) (0.1 * kSr); ++i) f.process(0.8f);
    float v = 0;
    for (int i = 0; i < (int) (0.1 * kSr); ++i) v = f.process(0.0f);
    REQUIRE(v > 0.05f);   // 100ms後もまだ残っている（release 200ms）
    for (int i = 0; i < (int) (2.0 * kSr); ++i) v = f.process(0.0f);
    REQUIRE(v < 0.01f);   // 2秒後はほぼゼロ
}

TEST_CASE("EnvFollower rectifies negative input") {
    raiu::EnvFollower f;
    f.setSampleRate(kSr);
    float v = 0;
    for (int i = 0; i < (int) (0.05 * kSr); ++i) v = f.process(-0.5f);
    REQUIRE(v == Catch::Approx(0.5f).margin(0.02f));
}
