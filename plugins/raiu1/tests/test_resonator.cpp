#include <catch2/catch_test_macros.hpp>
#include "dsp/texture/ResonatorBank.h"
#include "dsp/texture/TextureCommon.h"
#include <cmath>
#include <vector>

namespace {

constexpr double kSr = 48000.0;

// Goertzelアルゴリズムで特定周波数帯のエネルギーを測る
double goertzelPower(const std::vector<float>& buf, double hz, double sr) {
    double w = 2.0 * M_PI * hz / sr;
    double coeff = 2.0 * std::cos(w);
    double s0 = 0, s1 = 0, s2 = 0;
    for (auto x : buf) {
        s0 = (double) x + coeff * s1 - s2;
        s2 = s1;
        s1 = s0;
    }
    return s1 * s1 + s2 * s2 - coeff * s1 * s2;
}

std::vector<float> renderNoiseThrough(raiu::ResonatorBank& bank, int n, uint32_t seed = 99u) {
    raiu::tex::Rng rng{seed};
    std::vector<float> out((size_t) n);
    for (auto& s : out) s = bank.process(rng.bipolar());
    return out;
}

} // namespace

TEST_CASE("Resonator concentrates noise energy at note frequency") {
    raiu::ResonatorBank bank;
    bank.setSampleRate(kSr);
    bank.setResonance(60.0f);
    bank.noteOn(69);  // A4 = 440Hz
    auto buf = renderNoiseThrough(bank, (int) kSr * 2);
    double at440 = goertzelPower(buf, 440.0, kSr);
    double at1000 = goertzelPower(buf, 1000.0, kSr);
    REQUIRE(at440 > at1000 * 10.0);
}

TEST_CASE("Resonator is silent with no active notes") {
    raiu::ResonatorBank bank;
    bank.setSampleRate(kSr);
    auto buf = renderNoiseThrough(bank, (int) kSr);
    double acc = 0;
    for (auto s : buf) acc += (double) s * s;
    REQUIRE(std::sqrt(acc / buf.size()) < 1e-4);
}

TEST_CASE("Resonator fades out after noteOff") {
    raiu::ResonatorBank bank;
    bank.setSampleRate(kSr);
    bank.noteOn(69);
    renderNoiseThrough(bank, (int) kSr);
    bank.noteOff(69);
    renderNoiseThrough(bank, (int) kSr * 2);  // 2秒で減衰しきる
    auto tail = renderNoiseThrough(bank, 4800);
    double acc = 0;
    for (auto s : tail) acc += (double) s * s;
    REQUIRE(std::sqrt(acc / tail.size()) < 1e-3);
}

TEST_CASE("Resonator handles polyphony and stays stable at high Q") {
    raiu::ResonatorBank bank;
    bank.setSampleRate(kSr);
    bank.setResonance(120.0f);
    for (int note : {48, 52, 55, 59, 62, 65, 69, 72})
        bank.noteOn(note);
    auto buf = renderNoiseThrough(bank, (int) kSr * 10);
    for (auto s : buf) {
        REQUIRE(std::isfinite(s));
        REQUIRE(std::abs(s) <= 4.0f);
    }
}
