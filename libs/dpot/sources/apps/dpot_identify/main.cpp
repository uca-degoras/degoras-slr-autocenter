/***********************************************************************************************************************
 *  DegorasOpticalTracking_PoF - Degoras Optical Tracking: real-time detection of stars, satellites and the laser
 *  beam in the cameras of the SFEL station.
 *
 *  Developed as free software by and for the Spanish Navy Observatory SLR station (SFEL) in San Fernando.
 *
 *  Copyright (C) 2026 Degoras Project Team
 *
 *  This program is free software: you can redistribute it and/or modify it under the terms of the GNU General
 *  Public License as published by the Free Software Foundation, either version 3 of the License, or (at your
 *  option) any later version.
 *
 *  This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the
 *  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License
 *  for more details.
 *
 *  You should have received a copy of the GNU General Public License along with this program. If not, see
 *  <https://www.gnu.org/licenses/>.
 *
 *  SPDX-License-Identifier: GPL-3.0-or-later
 **********************************************************************************************************************/

/** @file main.cpp
 *  @brief dpot_identify: plate solves RASA frames and measures every FK5 star in them, the target one first of all.
 *
 *    dpot_identify [options] <d80 dir> <catalogs dir> <target FK5 number> <png>...
 *
 *      --hint           hinted solve: spiral search from the target (faster-astap's method); default blind, by index
 *      --radius <deg>   search radius of --hint (default 10)
 *      --fov <deg>      height of the frame (default 2.25: RASA 8 + ASI2600MM Pro)
 *      --bin <n>        software binning before detection (default 3)
 *      --stars <n>      stars handed to the solver (default 200)
 *      --crop <f>       solve on the central fraction f of each side (default 1)
 *      --sip 0|1        fit SIP distortion (default 1)
 *      --index <file>   quad index cache (default <d80 dir>/../../index/d80.idx; built the first time, ~4 s, 0.9 GB)
 *      --out <dir>      per frame: <name>.jpg with the solver's stars and the FK5 ones marked, <name>_target.png
 *
 *  Prints per frame the solution and the time of each step (the frame already in memory; reading the PNG apart), each
 *  FK5 star predicted, measured and their difference, and the target's computed Az/El and offset from the image centre
 *  in the mount's axes. Ends with the mean times and the target's scatter. Port of the experiment's fk5_fast.
 *  @author Degoras Project Team
 *  @copyright GPL-3.0-or-later
 */

// C++ INCLUDES
#include <algorithm>
#include <chrono>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <filesystem>
#include <memory>
#include <string>
#include <vector>

// PROJECT INCLUDES
#include "catalog/fk5.h"
#include "catalog/star_names.h"
#include "common/stats.h"
#include "io/canvas.h"
#include "io/capture_time.h"
#include "io/png_io.h"
#include "measure/star_measure.h"
#include "pointing/camera_model.h"
#include "solve/solver.h"

using namespace dpot;

namespace
{

using Clock = std::chrono::steady_clock;

double msSince(Clock::time_point t)
{
    return std::chrono::duration<double, std::milli>(Clock::now() - t).count();
}

struct Row
{
    const catalog::Fk5Star* star;
    double px, py;   // predicted, frame pixels
    bool found;
    measure::StarMeasurement m;
};

// The frame stretched, the solver's stars (green), the image centre (magenta), the FK5 stars (yellow, labelled) and
// the target (red, with its prediction in cyan); and a 4x crop around the target.
void writeAnnotated(const common::Frame& frame, const std::vector<detection::DetectedStar>& stars,
                    const std::vector<Row>& rows, const catalog::Fk5Star* target,
                    const std::unordered_map<int, std::string>& names, const std::string& base)
{
    std::vector<float> sample;
    for (int y = 0; y < frame.height; y += 7)
        for (int x = 0; x < frame.width; x += 7)
            sample.push_back(frame.at(x, y));
    const float bg = common::median(sample);
    io::Canvas cv = io::stretched(frame, bg, common::madSigma(sample, bg));
    for (const detection::DetectedStar& s : stars)
        cv.circle(s.x, s.y, 10, 2, io::colors::kGreen);
    cv.cross((frame.width - 1) / 2.0, (frame.height - 1) / 2.0, 40, 0, io::colors::kMagenta);
    const Row* found = nullptr;
    for (const Row& r : rows)
    {
        const bool t = r.star == target;
        cv.circle(r.found ? r.m.x : r.px, r.found ? r.m.y : r.py, t ? 60 : 35, t ? 4 : 3,
                  t ? io::colors::kRed : io::colors::kYellow);
        if (!t)
            continue;
        cv.cross(r.px, r.py, 12, 3, io::colors::kCyan);
        if (r.found)
        {
            cv.cross(r.m.x, r.m.y, 25, 0, io::colors::kRed);
            found = &r;
        }
    }
    if (found)
    {
        io::Canvas crop = cv.crop(found->m.x, found->m.y, 100, 4);   // before the labels, which would reach into it
        crop.text(12, 12, catalog::fk5Label(*found->star, names), 3, io::colors::kRed);
        crop.savePng(base + "_target.png");
    }
    constexpr int kScale = 5;
    for (const Row& r : rows)
    {
        const bool t = r.star == target;
        const std::string label = catalog::fk5Label(*r.star, names);
        const double x = r.found ? r.m.x : r.px, y = r.found ? r.m.y : r.py, rad = t ? 60 : 35;
        const double tw = io::Canvas::textWidth(label, kScale);
        cv.text(x + rad + 15 + tw > frame.width ? x - rad - 15 - tw : x + rad + 15, y - 10 * kScale, label, kScale,
                t ? io::colors::kRed : io::colors::kYellow);
    }
    cv.saveJpg(base + ".jpg");
}

} // namespace

int main(int argc, char** argv)
{
    solve::SolveRequest request;
    request.fovHeightDeg = 2.25;
    bool hint = false;
    double radius = 10;
    std::string indexPath, outDir;
    int a = 1;
    for (; a < argc && std::string(argv[a]).rfind("--", 0) == 0; a++)
    {
        const std::string k = argv[a];
        if (k == "--hint")
        {
            hint = true;
            continue;
        }
        if (a + 1 >= argc)
            break;
        const std::string v = argv[++a];
        if (k == "--radius") radius = std::atof(v.c_str());
        else if (k == "--fov") request.fovHeightDeg = std::atof(v.c_str());
        else if (k == "--bin") request.bin = std::atoi(v.c_str());
        else if (k == "--stars") request.maxStars = std::atoi(v.c_str());
        else if (k == "--crop") request.crop = std::atof(v.c_str());
        else if (k == "--sip") request.sip = v != "0";
        else if (k == "--index") indexPath = v;
        else if (k == "--out") outDir = v;
        else
        {
            std::fprintf(stderr, "unknown option %s\n", k.c_str());
            return 2;
        }
    }
    if (argc - a < 4)
    {
        std::fprintf(stderr, "usage: dpot_identify [--hint] [--radius deg] [--fov deg] [--bin n] [--stars n] "
                             "[--crop f] [--sip 0|1] [--index file] [--out dir] <d80 dir> <catalogs dir> "
                             "<target FK5> <png>...\n");
        return 2;
    }
    const std::string d80 = argv[a], catalogs = argv[a + 1];
    if (indexPath.empty())
        indexPath = (std::filesystem::path(d80).parent_path().parent_path() / "index" / "d80.idx").string();

    std::vector<catalog::Fk5Star> fk5;
    std::string error;
    if (!catalog::readFk5(catalogs, fk5, &error))
    {
        std::fprintf(stderr, "%s\n", error.c_str());
        return 2;
    }
    const catalog::Fk5Star* target = catalog::findFk5(fk5, std::atoi(argv[a + 2]));
    if (!target)
    {
        std::fprintf(stderr, "FK5 %s is not in %s\n", argv[a + 2], catalogs.c_str());
        return 2;
    }
    const auto names = catalog::readIauNames(catalogs + "/names/IAU-CSN.txt");
    if (!outDir.empty())
        std::filesystem::create_directories(outDir);

    solve::SolverConfig config;
    config.databaseDir = d80;
    config.indexFile = hint ? "" : indexPath;
    auto t = Clock::now();
    std::unique_ptr<solve::Solver> solver;
    if (const solve::Status s = solve::Solver::open(config, solver, &error); s != solve::Status::OK)
    {
        std::fprintf(stderr, "%s: %s\n", solve::toString(s), error.c_str());
        return 1;
    }
    std::printf("Solver ready in %.1f s (%s, index %.2f GB), target %s\n", msSince(t) / 1000,
                hint ? "hinted, spiral" : "blind", solver->indexGigabytes(), catalog::fk5Label(*target, names).c_str());

    const pointing::CameraModel camera = pointing::rasaCamera();
    const pointing::Site site = pointing::sfelSite();
    std::vector<double> tBin, tBg, tDetect, tSolve, tRefine, tMeasure, tTotal, tx, ty, td;
    int frames = 0, solved = 0;
    for (int i = a + 3; i < argc; i++)
    {
        frames++;
        t = Clock::now();
        common::Frame frame;
        common::UtcTime utc;
        const io::TimeSource source = io::captureTime(argv[i], utc);
        if (!io::loadFrame(argv[i], frame, &error) || source == io::TimeSource::NONE)
        {
            std::printf("\n== %s: skipped, %s\n", argv[i],
                        source == io::TimeSource::NONE ? "no capture time" : error.c_str());
            continue;
        }
        const double tPng = msSince(t);
        const double years = common::julianYearsSinceJ2000(utc);
        std::printf("\n== %s  %s (%s)\n", argv[i], common::toIso(utc).c_str(), io::toString(source));

        if (hint)
        {
            request.position = catalog::positionAt(*target, years);
            request.radiusDeg = radius;
        }
        const solve::SolveResult r = solver->solve(frame, request);
        if (r.status != solve::Status::OK)
        {
            std::printf("   not solved: %s %s\n", solve::toString(r.status), r.message.c_str());
            continue;
        }
        solved++;
        const auto t0 = Clock::now();
        std::vector<Row> rows;
        for (const catalog::Fk5Star& s : fk5)
        {
            double x, y;
            if (!wcs::skyToFrame(r.wcs, catalog::positionAt(s, years), x, y))
                continue;
            if (x < -0.5 || y < -0.5 || x > frame.width - 0.5 || y > frame.height - 0.5)
                continue;
            Row row{&s, x, y, false, {}};
            row.found = measure::measureStar(frame, x, y, row.m);
            rows.push_back(row);
        }
        tMeasure.push_back(msSince(t0));
        const solve::SolveMetrics& m = r.metrics;
        tBin.push_back(m.binMs);
        tSolve.push_back(m.solveMs);
        tTotal.push_back(m.totalMs + tMeasure.back());
        if (!hint)
        {
            tBg.push_back(m.backgroundMs);
            tDetect.push_back(m.detectMs);
            tRefine.push_back(m.refineMs);
            std::printf("   noise %.0f, %zu of %zu stars | bin %.1f, background %.1f, detect %.1f, solve %.1f, "
                        "refine %.1f", m.noise, m.starsUsed, m.candidates, m.binMs, m.backgroundMs, m.detectMs,
                        m.solveMs, m.refineMs);
        }
        else
            std::printf("   bin %.1f, solve %.1f", m.binMs, m.solveMs);
        std::printf(", measure %.2f | total %.1f ms (reading the PNG %.0f ms)\n", tMeasure.back(), tTotal.back(), tPng);
        const double scale = wcs::arcsecPerPixel(r.wcs);
        std::printf("   centre RA %.5f Dec %+.5f deg, %.4f\"/px, rotation %.2f deg%s\n", r.wcs.ra0 * common::kRadToDeg,
                    r.wcs.dec0 * common::kRadToDeg, scale, r.wcs.rotationDeg, r.wcs.sip.valid ? ", SIP" : "");
        std::printf("   FK5    Vmag   predicted x,y      measured x,y       dx      dy  dist\"\n");
        for (const Row& row : rows)
        {
            const std::string label = catalog::fk5Label(*row.star, names);
            std::string tail = label.find('\n') == std::string::npos ? "" : "  " + label.substr(label.find('\n') + 1);
            if (row.star == target)
                tail += "  <- target";
            std::printf("   %-6d %5.2f %8.1f,%8.1f ", row.star->number, row.star->vmag, row.px, row.py);
            if (!row.found)
            {
                std::printf("not found%s\n", tail.c_str());
                continue;
            }
            const double dx = row.m.x - row.px, dy = row.m.y - row.py, d = std::hypot(dx, dy) * scale;
            std::printf("%8.1f,%8.1f %7.2f %7.2f %6.2f  %s %d px saturated%s\n", row.m.x, row.m.y, dx, dy, d,
                        row.m.blob ? "blob," : ("SNR " + std::to_string(static_cast<int>(row.m.snr)) + ",").c_str(),
                        row.m.saturated, tail.c_str());
            if (row.star != target)
                continue;
            tx.push_back(row.m.x);
            ty.push_back(row.m.y);
            td.push_back(d);
            const pointing::HorizonPosition h = pointing::toHorizon(catalog::positionAt(*target, years), utc, site);
            const pointing::MountOffset o = pointing::starOffset(camera, row.m.x, row.m.y);
            std::printf("   target Az %.4f El %.4f deg (approximate, pointing/horizon.h); star - image centre: "
                        "dAz*cosEl %+.1f\", dEl %+.1f\"\n", h.azimuth * common::kRadToDeg,
                        h.elevation * common::kRadToDeg, o.dAzCosEl, o.dEl);
        }
        if (!outDir.empty())
            writeAnnotated(frame, r.stars, rows, target, names,
                           outDir + "/" + std::filesystem::path(argv[i]).stem().string());
    }

    std::printf("\n== %d of %d solved. Mean ms: bin %.1f", solved, frames, common::mean(tBin));
    if (!hint)
        std::printf(", background %.1f, detect %.1f, solve %.1f, refine %.1f", common::mean(tBg),
                    common::mean(tDetect), common::mean(tSolve), common::mean(tRefine));
    else
        std::printf(", solve %.1f", common::mean(tSolve));
    std::printf(", measure %.2f, total %.1f (max %.1f)\n", common::mean(tMeasure), common::mean(tTotal),
                tTotal.empty() ? 0 : *std::max_element(tTotal.begin(), tTotal.end()));
    std::printf("   target measured %zu times at x %.2f +- %.2f, y %.2f +- %.2f px, %.2f\" from the prediction "
                "(max %.2f\")\n", tx.size(), common::mean(tx), common::stdev(tx), common::mean(ty), common::stdev(ty),
                common::mean(td), td.empty() ? 0 : *std::max_element(td.begin(), td.end()));
    return solved == frames ? 0 : 1;
}
