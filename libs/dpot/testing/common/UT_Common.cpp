/***********************************************************************************************************************
 *  DegorasOpticalTracking_PoF - Degoras Optical Tracking: real-time detection of stars, satellites and the laser
 *  beam in the cameras of the SFEL station.
 *
 *  Copyright (C) 2026 Degoras Project Team
 *
 *  SPDX-License-Identifier: GPL-3.0-or-later
 **********************************************************************************************************************/

/** @file UT_Common.cpp
 *  @brief Unit test of the common layer: statistics, angle wrapping, Julian dates (Meeus, ch. 7), ISO times and the
 *         parallel for.
 */

// C++ INCLUDES
#include <atomic>
#include <random>
#include <vector>

// PROJECT INCLUDES
#include "common/angles.h"
#include "common/parallel.h"
#include "common/stats.h"
#include "common/utc_time.h"
#include "support/check.h"

using namespace dpot::common;

int main()
{
    // Statistics.
    CHECK(median({}) == 0);
    CHECK(median({3, 1, 2}) == 2);
    CHECK(median({4, 1, 3, 2}) == 3);   // the upper one
    std::mt19937 rng(7);
    std::normal_distribution<float> noise(100, 5);
    std::vector<float> v;
    for (int i = 0; i < 20000; i++)
        v.push_back(noise(rng));
    v.push_back(1e6f);   // a star does not move it
    const float m = median(v);
    CHECK_NEAR(m, 100, 0.2);
    CHECK_NEAR(madSigma(v, m), 5, 0.15);
    CHECK_NEAR(mean({1, 2, 3, 4}), 2.5, 1e-12);
    CHECK_NEAR(stdev({2, 4, 4, 4, 5, 5, 7, 9}), 2.138089935, 1e-8);
    CHECK_NEAR(slope({10, 10.5, 11, 11.5}), 0.5, 1e-12);

    // Angles.
    CHECK_NEAR(wrapPi(3 * kPi / 2), -kPi / 2, 1e-12);
    CHECK_NEAR(wrapTwoPi(-kPi / 2), 3 * kPi / 2, 1e-12);

    // Julian dates (Meeus, examples 7.a and the table of ch. 7).
    CHECK_NEAR(julianDate({1957, 10, 4, 19, 26, 24}), 2436116.31, 1e-6);
    CHECK_NEAR(julianDate({2000, 1, 1, 12, 0, 0}), 2451545.0, 1e-9);
    CHECK_NEAR(julianDate({1987, 6, 19, 12, 0, 0}), 2446966.0, 1e-9);
    CHECK_NEAR(julianDate({1600, 12, 31, 0, 0, 0}), 2305812.5, 1e-9);
    CHECK_NEAR(julianYearsSinceJ2000({2026, 10, 28, 19, 24, 41}), 26.8239, 1e-3);

    // ISO round trip.
    UtcTime t;
    CHECK(parseIso("2026-10-28T19:24:41.250Z", t));
    CHECK(t.year == 2026 && t.month == 10 && t.day == 28 && t.hour == 19 && t.minute == 24);
    CHECK_NEAR(t.second, 41.25, 1e-9);
    CHECK(toIso(t) == "2026-10-28T19:24:41.250Z");
    CHECK(!parseIso("2026-13-28T19:24:41Z", t));
    CHECK(!parseIso("not a time", t));

    // Every index exactly once.
    std::vector<std::atomic<int>> hits(1000);
    parallelFor(0, hits.size(), [&](std::size_t i) { hits[i]++; });
    for (const auto& h : hits)
        CHECK(h == 1);
    parallelFor(5, 5, [&](std::size_t) { CHECK(false); });
    return 0;
}
