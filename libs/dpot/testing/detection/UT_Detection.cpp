/***********************************************************************************************************************
 *  DegorasOpticalTracking_PoF - Degoras Optical Tracking: real-time detection of stars, satellites and the laser
 *  beam in the cameras of the SFEL station.
 *
 *  Copyright (C) 2026 Degoras Project Team
 *
 *  SPDX-License-Identifier: GPL-3.0-or-later
 **********************************************************************************************************************/

/** @file UT_Detection.cpp
 *  @brief Unit test of the detection layer on synthetic images: background grid, star detector (positions, hot pixels
 *         rejected), binning geometry, blobs (a star against a trail), Bayer sums and planes, field stop fit and the
 *         green excess of a green line.
 */

// C++ INCLUDES
#include <initializer_list>
#include <algorithm>
#include <cmath>
#include <random>
#include <vector>

// PROJECT INCLUDES
#include "detection/background.h"
#include "detection/bayer.h"
#include "detection/binning.h"
#include "detection/blobs.h"
#include "detection/field_stop.h"
#include "detection/green_excess.h"
#include "detection/star_detector.h"
#include "support/check.h"

using namespace dpot;

namespace
{

void addStar(common::FloatImage& img, double x0, double y0, double peak, double sigma)
{
    for (int y = static_cast<int>(y0) - 12; y <= static_cast<int>(y0) + 12; y++)
        for (int x = static_cast<int>(x0) - 12; x <= static_cast<int>(x0) + 12; x++)
            if (x >= 0 && y >= 0 && x < img.width && y < img.height)
                img.at(x, y) += static_cast<float>(peak * std::exp(-((x - x0) * (x - x0) + (y - y0) * (y - y0)) /
                                                                   (2 * sigma * sigma)));
}

} // namespace

int main()
{
    // A 700x500 field: background 1000, noise 10, 24 stars, 3 hot pixels.
    std::mt19937 rng(11);
    std::normal_distribution<float> noise(1000, 10);
    common::FloatImage img(700, 500);
    for (float& v : img.px)
        v = noise(rng);
    struct P { double x, y; };
    std::vector<P> stars;
    for (int k = 0; k < 24; k++)
    {
        const P p{40.3 + (k % 6) * 120 + 0.17 * k, 45.6 + (k / 6) * 120 - 0.11 * k};
        stars.push_back(p);
        addStar(img, p.x, p.y, 300 + 70 * k, 1.5);
    }
    for (const auto& [x, y] : {std::pair{100, 300}, {333, 222}, {600, 90}})
        img.at(x, y) += 5000;

    const detection::BackgroundGrid bg(img);
    CHECK(bg.nx == 6 && bg.ny == 4);
    for (std::size_t c = 0; c < bg.level.size(); c++)
    {
        CHECK_NEAR(bg.level[c], 1000, 3);
        CHECK_NEAR(bg.noise[c], 10, 1.5);
    }
    std::size_t candidates = 0;
    const std::vector<detection::DetectedStar> found = detection::detectStars(img, bg, {}, &candidates);
    CHECK(found.size() == stars.size());
    for (const P& p : stars)
    {
        double nearest = 1e9;
        for (const detection::DetectedStar& s : found)
            nearest = std::min(nearest, std::hypot(s.x - p.x, s.y - p.y));
        CHECK_NEAR(nearest, 0, 0.4);   // the 5x5 centroid of a sigma 1.5 star is biased up to ~0.3 px: good
                                       // enough to solve; stars are measured at full resolution afterwards
    }
    for (std::size_t i = 1; i < found.size(); i++)
        CHECK(found[i - 1].flux >= found[i].flux);   // brightest first
    detection::DetectorOptions few;
    few.maxStars = 5;
    CHECK(detection::detectStars(img, bg, few).size() == 5);

    // Binning: means of blocks, and where a binned pixel sits in the frame.
    common::Frame frame;
    frame.width = 10;
    frame.height = 7;
    for (int i = 0; i < 70; i++)
        frame.px.push_back(static_cast<std::uint16_t>(i));
    detection::BinnedGeometry geo;
    const common::FloatImage small = detection::binFrame(frame, 3, 1.0, geo);
    CHECK(small.width == 3 && small.height == 2 && geo.x0 == 0 && geo.y0 == 0);
    CHECK_NEAR(small.at(0, 0), (0 + 1 + 2 + 10 + 11 + 12 + 20 + 21 + 22) / 9.0, 1e-5);
    CHECK_NEAR(geo.frameX(0), 1, 1e-12);
    CHECK_NEAR(geo.frameY(1), 4, 1e-12);
    detection::binFrame(frame, 2, 0.5, geo);
    CHECK(geo.x0 == 3 && geo.y0 == 2);

    // Blobs: a round star and a satellite trail.
    common::FloatImage sky(300, 200, 100);
    addStar(sky, 60.4, 70.2, 500, 2);
    for (int x = 120; x < 260; x++)
        for (int y = 140; y < 143; y++)
            sky.at(x, y) += 300;
    const std::vector<detection::Blob> blobs = detection::findBlobs(sky, 100, 150);
    CHECK(blobs.size() == 2);
    CHECK(blobs[0].pixels == 140 * 3 && blobs[0].elongation > 20 && std::fabs(blobs[0].angleDeg) < 1);
    CHECK_NEAR(blobs[1].x, 60.4, 0.05);
    CHECK_NEAR(blobs[1].y, 70.2, 0.05);
    CHECK(blobs[1].elongation < 1.1);

    // Bayer: sums of 2x2 cells and RGGB planes, from the raw values of an 8 bit frame.
    common::Frame raw;
    raw.width = 4;
    raw.height = 2;
    raw.bitDepth = 8;
    for (int v : {10, 20, 30, 40, 50, 60, 70, 80})
        raw.px.push_back(static_cast<std::uint16_t>(v * 257));
    const common::FloatImage grey = detection::bayerSum(raw);
    CHECK(grey.width == 2 && grey.height == 1);
    CHECK(grey.at(0, 0) == 10 + 20 + 50 + 60 && grey.at(1, 0) == 30 + 40 + 70 + 80);
    const detection::ColourPlanes planes = detection::splitRggb(raw);
    CHECK(planes.r.at(0, 0) == 10 && planes.g.at(0, 0) == 35 && planes.b.at(0, 0) == 60);
    CHECK_NEAR(detection::greyToRaw(detection::rawToGrey(123.4)), 123.4, 1e-12);

    // Field stop: a lit disc of radius 230 centred at (410.5, 290.25) in a dark 800x600 image.
    common::FloatImage disc(800, 600, 5);
    for (int y = 0; y < 600; y++)
        for (int x = 0; x < 800; x++)
            if (std::hypot(x - 410.5, y - 290.25) <= 230)
                disc.at(x, y) = 100;
    const detection::FieldMask mask = detection::fieldMask(disc);
    double cx = 0, cy = 0, r = 0;
    CHECK(detection::fitFieldStop(mask, 800, 600, cx, cy, r));
    CHECK_NEAR(cx, 410.5, 0.3);
    CHECK_NEAR(cy, 290.25, 0.3);
    CHECK_NEAR(r, 230, 1);

    // Green excess: a line lit only in green stands out; a white star masked does not.
    detection::ColourPlanes c{disc, disc, disc};
    for (int x = 300; x < 520; x++)
        c.g.at(x, 300) += 20;
    addStar(c.r, 400, 220, 500, 2);
    addStar(c.g, 400, 220, 500, 2);
    addStar(c.b, 400, 220, 500, 2);
    double k = 0;
    const common::FloatImage e = detection::greenExcess(c, mask, 400, 220, 15, k);
    CHECK_NEAR(k, 1, 1e-3);
    CHECK(e.at(410, 300) > 0.5);
    CHECK(std::fabs(e.at(410, 250)) < 0.2);
    return 0;
}
