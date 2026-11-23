/***********************************************************************************************************************
 *  DegorasOpticalTracking_PoF - Degoras Optical Tracking: real-time detection of stars, satellites and the laser
 *  beam in the cameras of the SFEL station.
 *
 *  Copyright (C) 2026 Degoras Project Team
 *
 *  SPDX-License-Identifier: GPL-3.0-or-later
 **********************************************************************************************************************/

/** @file UT_Wcs.cpp
 *  @brief Unit test of the wcs layer: pixel <-> sky round trips, the reference pixel, the plate scale and the rewrite
 *         of a binned crop's solution for the whole frame.
 */

// C++ INCLUDES
#include <initializer_list>
#include <cmath>

// PROJECT INCLUDES
#include "wcs/wcs.h"
#include "support/check.h"

using namespace dpot;
using common::kDegToRad;

namespace
{

// A RASA-like solution: 1.932"/px, north turned 30 deg, mirrored in x like the ASICap frames.
wcs::Wcs rasaLike(int width, int height)
{
    wcs::Wcs w;
    w.ra0 = 340.72 * kDegToRad;
    w.dec0 = 30.2 * kDegToRad;
    w.crpix1 = width / 2.0 + 0.5;
    w.crpix2 = height / 2.0 + 0.5;
    const double s = 1.932 / 3600, c = std::cos(30 * kDegToRad), n = std::sin(30 * kDegToRad);
    w.cd11 = -s * c;
    w.cd12 = s * n;
    w.cd21 = s * n;
    w.cd22 = s * c;
    w.width = width;
    w.height = height;
    return w;
}

} // namespace

int main()
{
    const wcs::Wcs w = rasaLike(6248, 4176);
    CHECK_NEAR(wcs::arcsecPerPixel(w), 1.932, 1e-9);

    // The reference pixel is the tangent point.
    const common::RaDec c = wcs::frameToSky(w, w.crpix1 - 1, w.height - w.crpix2);
    CHECK_NEAR(c.ra, w.ra0, 1e-12);
    CHECK_NEAR(c.dec, w.dec0, 1e-12);

    // Round trips over the frame, corners included.
    for (const auto& [x, y] : {std::pair{0.0, 0.0}, {6247.0, 0.0}, {0.0, 4175.0}, {6247.0, 4175.0}, {1234.5, 3001.25}})
    {
        double u = 0, v = 0;
        CHECK(wcs::skyToFrame(w, wcs::frameToSky(w, x, y), u, v));
        CHECK_NEAR(u, x, 1e-6);
        CHECK_NEAR(v, y, 1e-6);
    }
    // Frame y grows downwards: with north up (cd22 > 0) the top row is north of the bottom one.
    CHECK(wcs::frameToSky(w, 3000, 0).dec > wcs::frameToSky(w, 3000, 4000).dec);
    // The antipode is behind the tangent plane.
    double u = 0, v = 0;
    CHECK(!wcs::skyToFrame(w, {w.ra0 + common::kPi, -w.dec0}, u, v));

    // A binned crop's solution, rewritten for the frame, puts every sky point where the binning geometry says.
    const int bin = 3, x0 = 312, y0 = 207, bw = 1874, bh = 1254;   // the central 90% of a RASA frame
    wcs::Wcs binned = rasaLike(bw, bh);
    binned.cd11 *= bin;
    binned.cd12 *= bin;
    binned.cd21 *= bin;
    binned.cd22 *= bin;
    const wcs::Wcs full = wcs::binnedToFrame(binned, bin, x0, y0, 6248, 4176);
    CHECK_NEAR(wcs::arcsecPerPixel(full), 1.932, 1e-9);
    for (const auto& [bu, bv] : {std::pair{0.0, 0.0}, {1873.0, 1253.0}, {400.5, 900.25}})
    {
        const common::RaDec sky = wcs::frameToSky(binned, bu, bv);
        double fx = 0, fy = 0;
        CHECK(wcs::skyToFrame(full, sky, fx, fy));
        CHECK_NEAR(fx, bu * bin + (bin - 1) / 2.0 + x0, 1e-6);
        CHECK_NEAR(fy, bv * bin + (bin - 1) / 2.0 + y0, 1e-6);
    }
    return 0;
}
