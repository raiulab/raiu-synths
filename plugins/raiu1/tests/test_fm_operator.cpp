#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>
#include "dsp/FMOperator.h"
#include <cmath>
#include <vector>

namespace {

constexpr double kSr = 48000.0;

// ゼロクロス（負→正）の数から周波数を推定する
double estimateFreq(const std::vector<float>& buf, double sr) {
    int crossings = 0;
    int first = -1, last = -1;
    for (size_t i = 1; i < buf.size(); ++i) {
        if (buf[i - 1] <= 0.0f && buf[i] > 0.0f) {
            ++crossings;
            if (first < 0) first = (int) i;
            last = (int) i;
        }
    }
    if (crossings < 2) return 0.0;
    return (double) (crossings - 1) * sr / (double) (last - first);
}

raiu::FMOperator makeOp(float ratio, float level) {
    raiu::FMOperator op;
    op.setSampleRate(kSr);
    op.setRatio(ratio);
    op.setLevel(level);
    op.setEnvParams(0.001f, 0.1f, 1.0f, 0.1f);  // 最速アタック・サスティン1
    return op;
}

std::vector<float> render(raiu::FMOperator& op, int n, float mod = 0.0f) {
    std::vector<float> buf((size_t) n);
    for (auto& s : buf) s = op.next(mod);
    return buf;
}

} // namespace

TEST_CASE("Operator with ratio 1 produces base frequency") {
    auto op = makeOp(1.0f, 1.0f);
    op.noteOn(440.0f);
    auto buf = render(op, 4800);
    REQUIRE(estimateFreq(buf, kSr) == Catch::Approx(440.0).margin(1.0));
}

TEST_CASE("Operator with ratio 2 doubles frequency") {
    auto op = makeOp(2.0f, 1.0f);
    op.noteOn(440.0f);
    auto buf = render(op, 4800);
    REQUIRE(estimateFreq(buf, kSr) == Catch::Approx(880.0).margin(2.0));
}

TEST_CASE("Fixed mode ignores note frequency") {
    auto op = makeOp(1.0f, 1.0f);
    op.setFixedFrequency(100.0f);
    op.noteOn(440.0f);
    auto buf = render(op, 9600);
    REQUIRE(estimateFreq(buf, kSr) == Catch::Approx(100.0).margin(0.5));
}

TEST_CASE("Level 0 gives silence") {
    auto op = makeOp(1.0f, 0.0f);
    op.noteOn(440.0f);
    for (auto s : render(op, 4800)) REQUIRE(s == 0.0f);
}

TEST_CASE("Modulation input changes the output") {
    auto opA = makeOp(1.0f, 1.0f);
    auto opB = makeOp(1.0f, 1.0f);
    opA.noteOn(440.0f);
    opB.noteOn(440.0f);
    bool differs = false;
    for (int i = 0; i < 4800; ++i) {
        float mod = 0.5f * std::sin(2.0 * M_PI * 220.0 * i / kSr);
        if (std::abs(opA.next(0.0f) - opB.next(mod)) > 1e-3f) differs = true;
    }
    REQUIRE(differs);
}

TEST_CASE("Feedback changes the output") {
    auto opA = makeOp(1.0f, 1.0f);
    auto opB = makeOp(1.0f, 1.0f);
    opB.setFeedback(0.3f);
    opA.noteOn(220.0f);
    opB.noteOn(220.0f);
    bool differs = false;
    for (int i = 0; i < 4800; ++i)
        if (std::abs(opA.next(0.0f) - opB.next(0.0f)) > 1e-3f) differs = true;
    REQUIRE(differs);
}

TEST_CASE("Operator output is finite and bounded") {
    auto op = makeOp(7.0f, 1.0f);
    op.setFeedback(1.0f);
    op.noteOn(2000.0f);
    for (auto s : render(op, 48000, 0.7f)) {
        REQUIRE(std::isfinite(s));
        REQUIRE(std::abs(s) <= 1.001f);
    }
}
