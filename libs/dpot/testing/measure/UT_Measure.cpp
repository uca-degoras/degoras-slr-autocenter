/***********************************************************************************************************************
 *  DegorasOpticalTracking_PoF - Degoras Optical Tracking: real-time detection of stars, satellites and the laser
 *  beam in the cameras of the SFEL station.
 *
 *  Copyright (C) 2026 Degoras Project Team
 *
 *  SPDX-License-Identifier: GPL-3.0-or-later
 **********************************************************************************************************************/

/** @file UT_Measure.cpp
 *  @brief Unit test of the measure layer on synthetic frames: a star measured from an offset prediction, a heavily
 *         saturated one (blob), nothing on pure noise or at the edge; and the single star of a raw Bayer frame.
 */

// C++ INCLUDES
#include <algorithm>
#include <array>
#include <initializer_list>
#include <cmath>
#include <random>

// PROJECT INCLUDES
#include "detection/bayer.h"
#include "measure/spot_measure.h"
#include "measure/star_measure.h"
#include "support/check.h"

using namespace dpot;

namespace
{

// A 16 bit frame: background 2000, noise 20, Gaussian stars clipped at 65535.
common::Frame field(int w, int h, std::initializer_list<std::array<double, 4>> stars)   // x, y, peak, sigma
{
    std::mt19937 rng(3);
    std::normal_distribution<double> noise(2000, 20);
    common::Frame f;
    f.width = w;
    f.height = h;
    f.px.resize(static_cast<std::size_t>(w) * static_cast<std::size_t>(h));
    for (int y = 0; y < h; y++)
        for (int x = 0; x < w; x++)
        {
            double v = noise(rng);
            for (const auto& s : stars)
                v += s[2] * std::exp(-((x - s[0]) * (x - s[0]) + (y - s[1]) * (y - s[1])) / (2 * s[3] * s[3]));
            f.px[static_cast<std::size_t>(y) * static_cast<std::size_t>(w) + static_cast<std::size_t>(x)] =
                static_cast<std::uint16_t>(std::clamp(v, 0.0, 65535.0));
        }
    return f;
}

} // namespace

int main()
{
    const common::Frame f = field(400, 400, {{200.3, 190.7, 20000, 2}, {150.6, 260.2, 400000, 4}});
    measure::StarMeasurement m;
    CHECK(measure::measureStar(f, 203, 188, m));   // 3 px off the true position
    CHECK(!m.blob && m.snr > 50);
    CHECK_NEAR(m.x, 200.3, 0.05);
    CHECK_NEAR(m.y, 190.7, 0.05);
    CHECK(measure::measureStar(f, 151, 259, m));
    CHECK(m.blob && m.saturated > 100);
    CHECK_NEAR(m.x, 150.6, 0.2);
    CHECK_NEAR(m.y, 260.2, 0.2);
    CHECK(!measure::measureStar(f, 300, 300, m));   // only noise there
    CHECK(!measure::measureStar(f, 50, 200, m));    // too close to the edge

    // A RAW8 Bayer frame: background 10, one star of sigma 8 raw px (FWHM 19 px, 1.4" as SFEL sees the seeing) at
    // (201.3, 151.8).
    common::Frame raw;
    raw.width = 400;
    raw.height = 300;
    raw.bitDepth = 8;
    std::mt19937 rng(5);
    std::normal_distribution<double> noise(10, 1.5);
    for (int y = 0; y < raw.height; y++)
        for (int x = 0; x < raw.width; x++)
        {
            const double r2 = (x - 201.3) * (x - 201.3) + (y - 151.8) * (y - 151.8);
            const double v = noise(rng) + 200 * std::exp(-r2 / 128);
            raw.px.push_back(static_cast<std::uint16_t>(std::clamp(std::lround(v), 0L, 255L) * 257));
        }
    const measure::SpotMeasurement s = measure::measureSpot(detection::bayerSum(raw));
    CHECK(s.found());
    CHECK(s.sources.size() == 1);
    CHECK_NEAR(detection::greyToRaw(s.winX), 201.3, 0.1);
    CHECK_NEAR(detection::greyToRaw(s.winY), 151.8, 0.1);
    CHECK_NEAR(2 * 2.3548 * s.sigma, 2.3548 * 8, 2);   // FWHM in raw px, from moments above 3 sigma
    CHECK(measure::countSaturated(raw, 201, 151, 20) == 0);
    return 0;
}
