#include <catch2/catch_test_macros.hpp>
#include "dsp/texture/NatureOscillator.h"
#include <cmath>
#include <vector>

namespace {

constexpr double kSr = 48000.0;

double goertzelPower(const std::vector<float>& buf, double hz, double sr) {
    double w = 2.0 * M_PI * hz / sr;
    double coeff = 2.0 * std::cos(w);
    double s1 = 0, s2 = 0;
    for (auto x : buf) {
        double s0 = (double) x + coeff * s1 - s2;
        s2 = s1;
        s1 = s0;
    }
    return s1 * s1 + s2 * s2 - coeff * s1 * s2;
}

raiu::NatureOscillator make(raiu::NatureOscillator::Mode mode, float keyTrack = 1.0f) {
    raiu::NatureOscillator osc;
    osc.setSampleRate(kSr);
    raiu::NatureOscillator::Params p;
    p.mode = mode;
    p.level = 0.8f;
    p.keyTrack = keyTrack;
    p.attack = 0.005f;
    p.sustain = 1.0f;
    p.release = 0.1f;
    osc.setParams(p);
    return osc;
}

std::vector<float> render(raiu::NatureOscillator& osc, int n) {
    std::vector<float> buf((size_t) n, 0.0f);
    osc.render(buf.data(), n);
    return buf;
}

float rms(const std::vector<float>& buf) {
    double acc = 0;
    for (auto s : buf) acc += (double) s * s;
    return (float) std::sqrt(acc / (double) buf.size());
}

} // namespace

TEST_CASE("NatureOsc Off mode is silent") {
    auto osc = make(raiu::NatureOscillator::Mode::Off);
    osc.noteOn(440.0f);
    REQUIRE(rms(render(osc, (int) kSr)) < 1e-6f);
    REQUIRE_FALSE(osc.isActive());
}

TEST_CASE("NatureOsc Wind tracks note pitch when keyTrack=1") {
    auto osc = make(raiu::NatureOscillator::Mode::Wind);
    osc.noteOn(440.0f);
    auto buf = render(osc, (int) kSr * 2);
    // 440Hzにエネルギーが集中し、無関係な622Hz（増4度）より大きい
    REQUIRE(goertzelPower(buf, 440.0, kSr) > goertzelPower(buf, 622.25, kSr) * 5.0);
}

TEST_CASE("NatureOsc Insects carrier follows the note") {
    auto osc = make(raiu::NatureOscillator::Mode::Insects);
    osc.noteOn(880.0f);
    auto buf = render(osc, (int) kSr * 2);
    REQUIRE(goertzelPower(buf, 880.0, kSr) > goertzelPower(buf, 620.0, kSr) * 5.0);
}

TEST_CASE("NatureOsc keyTrack=0 ignores the note") {
    auto low = make(raiu::NatureOscillator::Mode::Wind, 0.0f);
    auto high = make(raiu::NatureOscillator::Mode::Wind, 0.0f);
    low.noteOn(110.0f);
    high.noteOn(1760.0f);
    auto bufLow = render(low, (int) kSr * 2);
    auto bufHigh = render(high, (int) kSr * 2);
    // どちらもモード既定周波数（440Hz）付近で鳴る
    double pLow = goertzelPower(bufLow, 440.0, kSr);
    double pHigh = goertzelPower(bufHigh, 440.0, kSr);
    REQUIRE(pLow > 0.0);
    double ratio = pLow > pHigh ? pLow / pHigh : pHigh / pLow;
    REQUIRE(ratio < 5.0);  // 音程による差がない
}

TEST_CASE("NatureOsc envelope releases to silence and inactive") {
    auto osc = make(raiu::NatureOscillator::Mode::Water);
    osc.noteOn(330.0f);
    render(osc, (int) kSr);
    osc.noteOff();
    render(osc, (int) kSr);  // release 0.1s × 10
    REQUIRE_FALSE(osc.isActive());
    REQUIRE(rms(render(osc, 4800)) < 1e-4f);
}

TEST_CASE("NatureOsc lowpass filter darkens high-frequency modes") {
    auto open = make(raiu::NatureOscillator::Mode::Insects);
    auto dark = make(raiu::NatureOscillator::Mode::Insects);
    raiu::NatureOscillator::Params p;
    p.mode = raiu::NatureOscillator::Mode::Insects;
    p.level = 0.8f;
    p.attack = 0.005f;
    p.sustain = 1.0f;
    p.cutoff = 12000.0f;
    open.setParams(p);
    p.cutoff = 300.0f;
    dark.setParams(p);
    open.noteOn(2000.0f);
    dark.noteOn(2000.0f);
    REQUIRE(rms(render(dark, (int) kSr)) < rms(render(open, (int) kSr)) * 0.5f);
}

TEST_CASE("NatureOsc all modes are finite and bounded") {
    using Mode = raiu::NatureOscillator::Mode;
    for (auto mode : {Mode::Wind, Mode::Water, Mode::Birds, Mode::Insects, Mode::Rain}) {
        auto osc = make(mode);
        osc.noteOn(523.25f);
        for (auto s : render(osc, (int) kSr * 3)) {
            REQUIRE(std::isfinite(s));
            REQUIRE(std::abs(s) <= 1.5f);
        }
    }
}
