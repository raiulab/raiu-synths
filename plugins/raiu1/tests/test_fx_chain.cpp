#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>
#include "src/FXChain.h"
#include <cmath>

TEST_CASE("LookaheadPeakLimiter catches a transient without clipping") {
    constexpr double sampleRate = 48000.0;
    raiu::LookaheadPeakLimiter limiter;
    limiter.prepare({sampleRate, 512, 2});

    juce::AudioBuffer<float> buffer(2, 512);
    buffer.clear();
    buffer.setSample(0, 100, 4.0f);
    buffer.setSample(1, 100, -3.0f);
    limiter.process(buffer);

    float peak = 0.0f;
    for (int ch = 0; ch < buffer.getNumChannels(); ++ch)
        for (int i = 0; i < buffer.getNumSamples(); ++i) {
            REQUIRE(std::isfinite(buffer.getSample(ch, i)));
            peak = std::max(peak, std::abs(buffer.getSample(ch, i)));
        }

    REQUIRE(peak <= raiu::LookaheadPeakLimiter::getCeiling() + 1.0e-6f);
    REQUIRE(buffer.getSample(0, 100 + limiter.getLatencySamples())
            == Catch::Approx(raiu::LookaheadPeakLimiter::getCeiling()).margin(1.0e-5f));
}

TEST_CASE("LookaheadPeakLimiter is transparent below its ceiling apart from latency") {
    constexpr double sampleRate = 48000.0;
    raiu::LookaheadPeakLimiter limiter;
    limiter.prepare({sampleRate, 512, 2});

    juce::AudioBuffer<float> buffer(2, 512);
    buffer.clear();
    buffer.setSample(0, 20, 0.5f);
    buffer.setSample(1, 20, -0.25f);
    limiter.process(buffer);

    const int delayed = 20 + limiter.getLatencySamples();
    REQUIRE(buffer.getSample(0, delayed) == Catch::Approx(0.5f));
    REQUIRE(buffer.getSample(1, delayed) == Catch::Approx(-0.25f));
}
