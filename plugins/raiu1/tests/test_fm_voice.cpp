#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>
#include "dsp/FMVoiceEngine.h"
#include <cmath>
#include <vector>

namespace {

constexpr double kSr = 48000.0;

raiu::FMVoiceEngine makeVoice() {
    raiu::FMVoiceEngine v;
    v.setSampleRate(kSr);
    raiu::FMVoiceEngine::Params p;  // 既定パラメーター
    p.algorithm = 8;                // 純音（全並列）で周波数を測りやすく
    for (auto& op : p.ops) op.level = 0.0f;
    p.ops[0].level = 1.0f;
    p.ops[0].ratio = 1.0f;
    p.ops[0].attack = 0.001f; p.ops[0].decay = 0.1f; p.ops[0].sustain = 1.0f; p.ops[0].release = 0.05f;
    v.setParams(p);
    return v;
}

double estimateFreq(const std::vector<float>& buf, double sr) {
    int crossings = 0; int first = -1, last = -1;
    for (size_t i = 1; i < buf.size(); ++i)
        if (buf[i - 1] <= 0.0f && buf[i] > 0.0f) {
            ++crossings;
            if (first < 0) first = (int) i;
            last = (int) i;
        }
    if (crossings < 2) return 0.0;
    return (double) (crossings - 1) * sr / (double) (last - first);
}

std::vector<float> render(raiu::FMVoiceEngine& v, int n) {
    std::vector<float> buf((size_t) n, 0.0f);
    v.render(buf.data(), n);
    return buf;
}

float rms(const std::vector<float>& buf) {
    double acc = 0;
    for (auto s : buf) acc += (double) s * s;
    return (float) std::sqrt(acc / (double) buf.size());
}

} // namespace

TEST_CASE("Voice pitch: MIDI note 69 = 440Hz, note 57 = 220Hz") {
    auto v = makeVoice();
    v.noteOn(69, 1.0f);
    REQUIRE(estimateFreq(render(v, 4800), kSr) == Catch::Approx(440.0).margin(1.0));

    auto v2 = makeVoice();
    v2.noteOn(57, 1.0f);
    REQUIRE(estimateFreq(render(v2, 4800), kSr) == Catch::Approx(220.0).margin(1.0));
}

TEST_CASE("Velocity sensitivity halves output at half velocity") {
    auto full = makeVoice();
    auto half = makeVoice();
    raiu::FMVoiceEngine::Params p;
    p.algorithm = 8;
    for (auto& op : p.ops) op.level = 0.0f;
    p.ops[0].level = 1.0f;
    p.ops[0].velSens = 1.0f;
    p.ops[0].attack = 0.001f; p.ops[0].sustain = 1.0f;
    full.setParams(p);
    half.setParams(p);
    full.noteOn(69, 1.0f);
    half.noteOn(69, 0.5f);
    float rFull = rms(render(full, 4800));
    float rHalf = rms(render(half, 4800));
    REQUIRE(rHalf == Catch::Approx(0.5f * rFull).epsilon(0.02f));
}

TEST_CASE("velSens 0 means velocity has no effect") {
    auto full = makeVoice();
    auto half = makeVoice();
    full.noteOn(69, 1.0f);
    half.noteOn(69, 0.3f);
    REQUIRE(rms(render(half, 4800)) == Catch::Approx(rms(render(full, 4800))).epsilon(0.01f));
}

TEST_CASE("Key scaling lowers level for higher notes") {
    raiu::FMVoiceEngine::Params p;
    p.algorithm = 8;
    for (auto& op : p.ops) op.level = 0.0f;
    p.ops[0].level = 1.0f;
    p.ops[0].keyScaleDbPerOct = 6.0f;  // 1オクターブで-6dB
    p.ops[0].attack = 0.001f; p.ops[0].sustain = 1.0f;

    raiu::FMVoiceEngine low, high;
    low.setSampleRate(kSr); high.setSampleRate(kSr);
    low.setParams(p); high.setParams(p);
    low.noteOn(60, 1.0f);
    high.noteOn(72, 1.0f);
    float rLow = rms(render(low, 4800));
    float rHigh = rms(render(high, 4800));
    REQUIRE(rHigh == Catch::Approx(0.5f * rLow).epsilon(0.03f));  // -6dB = 半分
}

TEST_CASE("Voice becomes inactive and silent after noteOff") {
    auto v = makeVoice();
    v.noteOn(69, 1.0f);
    render(v, 4800);
    v.noteOff();
    render(v, 48000);
    REQUIRE_FALSE(v.isActive());
    auto tail = render(v, 100);
    for (auto s : tail) REQUIRE(std::abs(s) < 1e-4f);
}

TEST_CASE("Portamento glides smoothly to target frequency") {
    auto v = makeVoice();
    v.setPortamentoTime(0.05f);
    v.noteOn(57, 1.0f);       // 220Hz
    render(v, 4800);
    v.noteOn(69, 1.0f);       // → 440Hzへグライド（レガート）
    auto early = render(v, 480);   // 10ms後: まだ途中
    render(v, 48000);              // 1秒後: 到達済み
    auto settled = render(v, 4800);
    double fEarly = estimateFreq(early, kSr);
    REQUIRE(fEarly > 225.0);
    REQUIRE(fEarly < 435.0);
    REQUIRE(estimateFreq(settled, kSr) == Catch::Approx(440.0).margin(2.0));
}

TEST_CASE("Pitch offset +200 cents raises frequency by 2 semitones") {
    auto v = makeVoice();
    v.noteOn(69, 1.0f);
    v.setPitchOffsetCents(200.0f);
    auto buf = render(v, 4800);
    REQUIRE(estimateFreq(buf, kSr)
            == Catch::Approx(440.0 * std::pow(2.0, 2.0 / 12.0)).margin(1.5));
}

TEST_CASE("Pitch offset 0 leaves frequency unchanged") {
    auto v = makeVoice();
    v.noteOn(69, 1.0f);
    v.setPitchOffsetCents(50.0f);
    v.setPitchOffsetCents(0.0f);
    REQUIRE(estimateFreq(render(v, 4800), kSr) == Catch::Approx(440.0).margin(1.0));
}

TEST_CASE("Modulator boost alters modulated timbre but not pure carriers") {
    // アルゴリズム1（直列）: モジュレーターのブーストで音色が変わる
    raiu::FMVoiceEngine::Params p;
    p.algorithm = 1;
    for (auto& op : p.ops) {
        op.level = 0.6f;
        op.attack = 0.001f; op.sustain = 1.0f;
    }
    raiu::FMVoiceEngine a, b;
    a.setSampleRate(kSr); b.setSampleRate(kSr);
    a.setParams(p); b.setParams(p);
    a.noteOn(60, 1.0f); b.noteOn(60, 1.0f);
    b.setModulatorBoost(1.4f);
    auto bufA = render(a, 4800), bufB = render(b, 4800);
    bool differs = false;
    for (size_t i = 0; i < bufA.size(); ++i)
        if (std::abs(bufA[i] - bufB[i]) > 1e-3f) differs = true;
    REQUIRE(differs);

    // キャリアのみ（アルゴリズム8のop1単体）はブーストの影響を受けない
    auto c = makeVoice(), d = makeVoice();
    c.noteOn(60, 1.0f); d.noteOn(60, 1.0f);
    d.setModulatorBoost(1.4f);
    auto bufC = render(c, 4800), bufD = render(d, 4800);
    for (size_t i = 0; i < bufC.size(); ++i)
        REQUIRE(bufC[i] == Catch::Approx(bufD[i]).margin(1e-6));
}

TEST_CASE("Nature oscillator mixes into the voice at note pitch") {
    auto v = makeVoice();
    raiu::FMVoiceEngine::Params p;
    for (auto& op : p.ops) op.level = 0.0f;  // FMは無音
    v.setParams(p);

    raiu::NatureOscillator::Params np;
    np.mode = raiu::NatureOscillator::Mode::Insects;  // 正弦キャリア=音程測定しやすい
    np.level = 0.8f;
    np.keyTrack = 1.0f;
    np.attack = 0.005f;
    np.sustain = 1.0f;
    v.setNatureParams(np);

    v.noteOn(69, 1.0f);
    REQUIRE(v.isActive());  // FMが無音でもNATUREが生きている
    auto buf = render(v, (int) kSr);
    double acc = 0;
    for (auto s : buf) acc += (double) s * s;
    REQUIRE(std::sqrt(acc / buf.size()) > 0.01);

    // Goertzelで440Hzにエネルギー集中を確認
    double w = 2.0 * M_PI * 440.0 / kSr, coeff = 2.0 * std::cos(w);
    double s1 = 0, s2 = 0;
    for (auto x : buf) { double s0 = (double) x + coeff * s1 - s2; s2 = s1; s1 = s0; }
    double p440 = s1 * s1 + s2 * s2 - coeff * s1 * s2;
    w = 2.0 * M_PI * 620.0 / kSr; coeff = 2.0 * std::cos(w);
    s1 = 0; s2 = 0;
    for (auto x : buf) { double s0 = (double) x + coeff * s1 - s2; s2 = s1; s1 = s0; }
    double p620 = s1 * s1 + s2 * s2 - coeff * s1 * s2;
    REQUIRE(p440 > p620 * 5.0);
}

TEST_CASE("Voice output is finite") {
    auto v = makeVoice();
    v.noteOn(100, 1.0f);
    for (auto s : render(v, 48000)) REQUIRE(std::isfinite(s));
}
