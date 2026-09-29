#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>
#include "dsp/SineTable.h"
#include <cmath>

TEST_CASE("SineTable matches std::sin within 1e-4") {
    for (double p = 0.0; p < 1.0; p += 0.001)
        REQUIRE(raiu::SineTable::lookup((float) p)
                == Catch::Approx(std::sin(p * 2.0 * M_PI)).margin(1e-4));
}

TEST_CASE("SineTable handles phase outside [0,1)") {
    REQUIRE(raiu::SineTable::lookup(1.25f)
            == Catch::Approx(raiu::SineTable::lookup(0.25f)).margin(1e-6));
    REQUIRE(std::isfinite(raiu::SineTable::lookup(-0.3f)));
    REQUIRE(raiu::SineTable::lookup(-0.3f)
            == Catch::Approx(raiu::SineTable::lookup(0.7f)).margin(1e-6));
}
