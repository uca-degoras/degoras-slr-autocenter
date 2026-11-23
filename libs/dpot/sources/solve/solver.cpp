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

/** @file solver.cpp
 *  @brief Implementation of solver.h on faster-astap (thirdparty/faster-astap).
 *  @author Degoras Project Team
 *  @copyright GPL-3.0-or-later
 */

// C++ INCLUDES
#include <algorithm>
#include <chrono>
#include <exception>
#include <filesystem>
#include <utility>

// THIRD PARTY INCLUDES
#include "astap/index_solver.h"
#include "astap/quad_index.h"
#include "astap/spiral_service.h"
#include "astap/star_database.h"
#include "astap/types.h"

// PROJECT INCLUDES
#include "common/stats.h"
#include "detection/background.h"
#include "detection/binning.h"
#include "solve/solver.h"

// NAMESPACES
namespace dpot::solve
{

namespace
{

using Clock = std::chrono::steady_clock;

double msSince(Clock::time_point t)
{
    return std::chrono::duration<double, std::milli>(Clock::now() - t).count();
}

wcs::Sip toSip(const astap::SipCoefficients& s)
{
    wcs::Sip out;
    out.valid = s.valid;
    out.aOrder = s.a_order;
    out.apOrder = s.ap_order;
    for (int p = 0; p < 4; p++)
        for (int q = 0; q < 4; q++)
        {
            out.a[p][q] = s.a[p][q];
            out.b[p][q] = s.b[p][q];
            out.ap[p][q] = s.ap[p][q];
            out.bp[p][q] = s.bp[p][q];
        }
    return out;
}

template <class Solution>
void copyLinear(const Solution& s, wcs::Wcs& w)
{
    w.ra0 = s.ra0;
    w.dec0 = s.dec0;
    w.crpix1 = s.crpix1;
    w.crpix2 = s.crpix2;
    w.cd11 = s.cd1_1;
    w.cd12 = s.cd1_2;
    w.cd21 = s.cd2_1;
    w.cd22 = s.cd2_2;
    w.rotationDeg = s.crota2;
}

} // namespace

// ---------------------------------------------------------------------------------------------------------------------
// IMPLEMENTATION

struct Solver::Impl
{
    astap::StarDatabase db;
    astap::SpiralService spiral;
    std::vector<astap::QuadIndex> tiers;
};

Solver::Solver() : impl_(std::make_unique<Impl>()) {}

Solver::~Solver() = default;

const char* toString(Status status) noexcept
{
    switch (status)
    {
        case Status::OK: return "OK";
        case Status::NO_SOLUTION: return "NO_SOLUTION";
        case Status::TOO_FEW_STARS: return "TOO_FEW_STARS";
        case Status::INVALID_ARGUMENT: return "INVALID_ARGUMENT";
        case Status::IMAGE_READ: return "IMAGE_READ";
        case Status::CATALOG_NOT_FOUND: return "CATALOG_NOT_FOUND";
        case Status::CATALOG_READ: return "CATALOG_READ";
        default: return "INTERNAL";
    }
}

Status Solver::open(const SolverConfig& config, std::unique_ptr<Solver>& out, std::string* message) noexcept
{
    auto fail = [message](Status s, const std::string& text) {
        if (message)
            *message = text;
        return s;
    };
    try
    {
        std::unique_ptr<Solver> s(new Solver());
        if (!s->impl_->db.select(config.databaseDir + "/", config.database, 1.0))
            return fail(Status::CATALOG_NOT_FOUND, "no " + config.database + " star database in " + config.databaseDir);
        astap::SpiralServiceSettings ss;
        ss.database_path = config.databaseDir;
        ss.database = config.database;
        if (!s->impl_->spiral.load(ss))
            return fail(Status::CATALOG_READ, "cannot load the " + config.database + " database for the spiral search");
        if (!config.indexFile.empty() && !astap::load_index_file(config.indexFile, s->impl_->tiers))
        {
            s->impl_->tiers.clear();
            astap::QuadIndexSettings qs;
            if (!astap::build_tiers(s->impl_->db, qs, config.indexTiers, s->impl_->tiers))
                return fail(Status::CATALOG_READ, "cannot build the quad index from " + config.databaseDir);
            const std::filesystem::path dir = std::filesystem::path(config.indexFile).parent_path();
            std::error_code ec;
            if (!dir.empty())
                std::filesystem::create_directories(dir, ec);
            if (!astap::save_index_file(config.indexFile, s->impl_->tiers) && message)
                *message = "index built, but it could not be cached in " + config.indexFile;
        }
        out = std::move(s);
        return Status::OK;
    }
    catch (const std::exception& e)
    {
        return fail(Status::INTERNAL, e.what());
    }
    catch (...)
    {
        return fail(Status::INTERNAL, "unknown exception");
    }
}

double Solver::indexGigabytes() const noexcept
{
    double bytes = 0;
    for (const astap::QuadIndex& ix : impl_->tiers)
        bytes += static_cast<double>(ix.bytes());
    return bytes / 1e9;
}

SolveResult Solver::solve(const common::Frame& frame, const SolveRequest& request) noexcept
{
    SolveResult res;
    const auto t0 = Clock::now();
    try
    {
        const bool blind = !request.position || request.radiusDeg >= 180;
        if (frame.width <= 0 || frame.height <= 0 || request.fovHeightDeg <= 0 || request.bin < 1 ||
            request.crop <= 0 || request.crop > 1 || request.maxStars < 4)
        {
            res.status = Status::INVALID_ARGUMENT;
            res.message = "empty frame, or fovHeightDeg <= 0, bin < 1, crop outside (0, 1] or maxStars < 4";
            return res;
        }
        if (blind && impl_->tiers.empty())
        {
            res.status = Status::INVALID_ARGUMENT;
            res.message = "a blind solve needs the quad index (SolverConfig::indexFile)";
            return res;
        }

        auto t = Clock::now();
        detection::BinnedGeometry geo;
        common::FloatImage small = detection::binFrame(frame, request.bin, request.crop, geo);
        res.metrics.binMs = msSince(t);
        const int sw = small.width, sh = small.height;
        // Height in degrees of what is solved: the crop of the frame.
        const double fov = request.fovHeightDeg * sh * request.bin / frame.height;
        wcs::Wcs binned;
        binned.width = sw;
        binned.height = sh;

        if (blind)
        {
            t = Clock::now();
            const detection::BackgroundGrid background(small);
            res.metrics.backgroundMs = msSince(t);
            res.metrics.noise = common::median(background.noise);
            t = Clock::now();
            detection::DetectorOptions options;
            options.maxStars = request.maxStars;
            const std::vector<detection::DetectedStar> stars =
                detection::detectStars(small, background, options, &res.metrics.candidates);
            res.metrics.detectMs = msSince(t);
            res.metrics.starsUsed = stars.size();
            if (stars.size() < 4)
            {
                res.status = Status::TOO_FEW_STARS;
                res.message = std::to_string(stars.size()) + " stars detected";
                return res;
            }
            // faster-astap's convention: rows bottom up.
            astap::RowList list(3, stars.size());
            for (std::size_t i = 0; i < stars.size(); i++)
            {
                list(0, i) = stars[i].x;
                list(1, i) = sh - 1 - stars[i].y;
                list(2, i) = stars[i].snr;
            }
            t = Clock::now();
            astap::IndexSolveResult r = astap::solve_stars_with_tiers(
                impl_->tiers, list, sw, sh, {}, static_cast<double>(stars.size()) / (fov * fov * sw / sh));
            res.metrics.solveMs = msSince(t);
            if (!r.solved)
            {
                res.status = Status::NO_SOLUTION;
                res.message = r.reason;
                return res;
            }
            t = Clock::now();
            const astap::IndexRefineResult ref =
                astap::refine_with_database(impl_->db, list, sw, sh, r, {}, request.sip);
            res.metrics.refineMs = msSince(t);
            copyLinear(r, binned);
            if (ref.sip_valid)
                binned.sip = toSip(ref.sip);
            for (const detection::DetectedStar& s : stars)
                res.stars.push_back({geo.frameX(s.x), geo.frameY(s.y), s.flux, s.snr});
        }
        else
        {
            astap::ImageArray image(1, sh, sw);
            for (int y = 0; y < sh; y++)
                std::copy(small.row(y), small.row(y) + sw, image.row(0, sh - 1 - y));
            astap::Header head;
            head.bitpix = 16;
            head.naxis = 2;
            head.naxis3 = 1;
            head.datamax_org = 65535;
            head.width = sw;
            head.height = sh;
            astap::SpiralParams p;
            p.ra = request.position->ra;
            p.dec = request.position->dec;
            p.fov = fov;
            p.fov_specified = true;
            p.radius = request.radiusDeg;
            p.downsample = 1;
            p.max_stars = request.maxStars;
            p.want_sip = request.sip;
            t = Clock::now();
            const astap::SolveOutcome o = impl_->spiral.solve_image(std::move(image), head, p);
            res.metrics.solveMs = msSince(t);
            if (!o.solved)
            {
                res.status = o.errorlevel == astap::kErrNotEnoughStars ? Status::TOO_FEW_STARS : Status::NO_SOLUTION;
                res.message = "the spiral search found no solution within " + std::to_string(request.radiusDeg) +
                              " deg";
                return res;
            }
            copyLinear(o.head, binned);
            binned.sip = toSip(o.sip);
        }
        res.wcs = wcs::binnedToFrame(binned, request.bin, geo.x0, geo.y0, frame.width, frame.height);
        res.status = Status::OK;
    }
    catch (const std::exception& e)
    {
        res.status = Status::INTERNAL;
        res.message = e.what();
    }
    catch (...)
    {
        res.status = Status::INTERNAL;
        res.message = "unknown exception";
    }
    res.metrics.totalMs = msSince(t0);
    return res;
}

} // namespace dpot::solve
