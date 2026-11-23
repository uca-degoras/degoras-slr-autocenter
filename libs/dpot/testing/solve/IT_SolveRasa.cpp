/***********************************************************************************************************************
 *  DegorasOpticalTracking_PoF - Degoras Optical Tracking: real-time detection of stars, satellites and the laser
 *  beam in the cameras of the SFEL station.
 *
 *  Copyright (C) 2026 Degoras Project Team
 *
 *  SPDX-License-Identifier: GPL-3.0-or-later
 **********************************************************************************************************************/

/** @file IT_SolveRasa.cpp
 *  @brief Integration test on the real RASA 8 frames (DPOT_DATA_DIR/images/RASA-8_ASI2600MMPRO) and the D80 star
 *         database (DPOT_DATA_DIR/databases/d80): the first frame of each of the four targets is solved blind (the
 *         first run builds the quad index in DPOT_DATA_DIR/index, ~4 s, 0.9 GB), the plate scale is the RASA's, and
 *         the target FK5 star is measured where the solution puts it. One frame is solved hinted too, and the two
 *         solutions must agree. Skipped (77) without the data.
 */

// C++ INCLUDES
#include <initializer_list>
#include <algorithm>
#include <cmath>
#include <cstdio>
#include <filesystem>
#include <string>
#include <vector>

// PROJECT INCLUDES
#include "catalog/fk5.h"
#include "io/capture_time.h"
#include "io/png_io.h"
#include "measure/star_measure.h"
#include "solve/solver.h"
#include "support/check.h"

using namespace dpot;
namespace fs = std::filesystem;

namespace
{

std::string firstPng(const fs::path& dir)
{
    std::vector<std::string> pngs;
    for (const auto& e : fs::recursive_directory_iterator(dir))
        if (e.path().extension() == ".PNG" || e.path().extension() == ".png")
            pngs.push_back(e.path().string());
    std::sort(pngs.begin(), pngs.end());
    return pngs.empty() ? "" : pngs.front();
}

} // namespace

int main()
{
    const std::string data = DPOT_DATA_DIR;
    const fs::path images = fs::path(data) / "images" / "RASA-8_ASI2600MMPRO";
    const std::string d80 = data + "/databases/d80";
    testing::requireData(images.string(), "RASA images");
    testing::requireData(d80 + "/d80_0101.1476", "D80 star database");

    std::vector<catalog::Fk5Star> fk5;
    CHECK(catalog::readFk5(std::string(DPOT_SOURCE_DIR) + "/data/catalogs", fk5));
    solve::SolverConfig config;
    config.databaseDir = d80;
    config.indexFile = data + "/index/d80.idx";
    std::unique_ptr<solve::Solver> solver;
    std::string message;
    CHECK(solve::Solver::open(config, solver, &message) == solve::Status::OK);

    solve::SolveRequest request;
    request.fovHeightDeg = 2.25;
    struct Target { const char* folder; int fk5; };
    for (const Target& t :
         {Target{"ALTAIR", 745}, Target{"FK51481", 1481}, Target{"MATAR", 857}, Target{"POLARIS", 907}})
    {
        const std::string png = firstPng(images / t.folder);
        CHECK(!png.empty());
        common::Frame frame;
        CHECK(io::loadFrame(png, frame));
        common::UtcTime utc;
        CHECK(io::captureTime(png, utc) != io::TimeSource::NONE);
        const solve::SolveResult r = solver->solve(frame, request);
        std::printf("%s: %s %s, %.4f\"/px, %zu stars, %.1f ms\n", t.folder, solve::toString(r.status),
                    r.message.c_str(), wcs::arcsecPerPixel(r.wcs), r.metrics.starsUsed, r.metrics.totalMs);
        CHECK(r.status == solve::Status::OK);
        CHECK_NEAR(wcs::arcsecPerPixel(r.wcs), 1.932, 0.01);
        CHECK(r.wcs.width == frame.width && r.wcs.height == frame.height && !r.stars.empty());

        // The target, predicted and measured: within 8" (saturated stars are the worst, 2-4" in the experiment).
        const catalog::Fk5Star* star = catalog::findFk5(fk5, t.fk5);
        CHECK(star != nullptr);
        const common::RaDec at = catalog::positionAt(*star, common::julianYearsSinceJ2000(utc));
        double x = 0, y = 0;
        CHECK(wcs::skyToFrame(r.wcs, at, x, y));
        measure::StarMeasurement m;
        CHECK(measure::measureStar(frame, x, y, m));
        const double off = std::hypot(m.x - x, m.y - y) * wcs::arcsecPerPixel(r.wcs);
        std::printf("   FK5 %d predicted %.1f, %.1f, measured %.1f, %.1f: %.2f\"%s\n", t.fk5, x, y, m.x, m.y, off,
                    m.blob ? " (blob)" : "");
        CHECK(off < 8);

        if (std::string(t.folder) != "MATAR")
            continue;
        // Hinted from the target, 10 deg: the same field to within 3".
        solve::SolveRequest hinted = request;
        hinted.position = at;
        hinted.radiusDeg = 10;
        const solve::SolveResult h = solver->solve(frame, hinted);
        CHECK(h.status == solve::Status::OK);
        const common::RaDec a = wcs::frameToSky(r.wcs, frame.width / 2.0, frame.height / 2.0);
        const common::RaDec b = wcs::frameToSky(h.wcs, frame.width / 2.0, frame.height / 2.0);
        const double sep = std::hypot((a.ra - b.ra) * std::cos(a.dec), a.dec - b.dec) * common::kRadToArcsec;
        std::printf("   hinted: %.1f ms, centre %.2f\" from the blind one\n", h.metrics.totalMs, sep);
        CHECK(sep < 3);
    }
    return 0;
}
