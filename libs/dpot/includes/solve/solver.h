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

/** @file solver.h
 *  @brief Plate solving: where in the sky a frame points, as a Wcs.
 *
 *  THE SEAM. This is the only door to faster-astap (cmake/dp_check_layers.cmake enforces it): nothing outside
 *  sources/solve and sources/measure includes it, and this header shows none of its types. The names follow the API
 *  planned for DegorasAstrometry (Status values = ASTAP's exit codes, SolveRequest with a position and a radius, 180 =
 *  blind), so when that library exists it replaces faster-astap behind this header and nothing else changes.
 *
 *  Two ways to solve, chosen by the request:
 *   - BLIND (no position, or radius 180): our binning + one pass detector on the frame, then faster-astap's quad index
 *     (a ladder of depth tiers built once from the D80 database, ~4 s, cached in a 0.9 GB file) and a second pass
 *     against the database with SIP. 12-17 ms per RASA frame in memory, no hint needed.
 *   - HINTED (position and radius < 180): faster-astap's spiral search from that position, with its own detector
 *     (find_stars). ASTAP's own method; ~0.1 s per frame.
 *  @author Degoras Project Team
 *  @copyright GPL-3.0-or-later
 */

#pragma once

// C++ INCLUDES
#include <cstdint>
#include <memory>
#include <optional>
#include <string>
#include <vector>

// PROJECT INCLUDES
#include "common/angles.h"
#include "common/frame.h"
#include "detection/star_detector.h"
#include "wcs/wcs.h"

// NAMESPACES
namespace dpot::solve
{

// ---------------------------------------------------------------------------------------------------------------------
// TYPES

/// Stable values; those ASTAP has are its exit codes.
enum class Status : std::uint8_t
{
    OK = 0,
    NO_SOLUTION = 1,
    TOO_FEW_STARS = 2,
    INVALID_ARGUMENT = 3,
    IMAGE_READ = 16,
    CATALOG_NOT_FOUND = 32,
    CATALOG_READ = 33,
    INTERNAL = 255
};

const char* toString(Status status) noexcept;

struct SolverConfig
{
    std::string databaseDir;   ///< Folder holding the ASTAP star database files (d80_*.1476)
    std::string database = "d80";
    /// Quad index cache for blind solves: read if it exists, else built from the database (~4 s) and written there.
    /// Empty: no index, only hinted solves.
    std::string indexFile;
    /// Index depth tiers in stars per square degree. The RASA frames give roughly 10-100 detected stars/deg2.
    /// ponytail: a slice of faster-astap's default ladder (0.5 .. 900, 2.7 GB); widen it for other fields or depths.
    std::vector<double> indexTiers = {4, 8, 16, 32, 60, 125, 250};
};

struct SolveRequest
{
    std::optional<common::RaDec> position;   ///< Where to look first (J2000, rad); none = blind
    double radiusDeg = 180;                  ///< Search radius around `position`; 180 = blind
    double fovHeightDeg = 0;                 ///< Height of the whole frame in degrees (RASA: 2.25). Required.
    int bin = 3;                             ///< Software binning before detection
    int maxStars = 200;                      ///< Stars handed to the solver
    double crop = 1;                         ///< Solve on the central fraction of each side
    bool sip = true;                         ///< Fit SIP distortion (blind: second pass against the database)
};

struct SolveMetrics
{
    std::size_t candidates = 0;   ///< Peaks found before the brightest maxStars were kept (blind)
    std::size_t starsUsed = 0;    ///< Stars handed to the solver (blind)
    double noise = 0;             ///< Median background noise of the binned frame (blind)
    double binMs = 0, backgroundMs = 0, detectMs = 0, solveMs = 0, refineMs = 0, totalMs = 0;
};

struct SolveResult
{
    Status status = Status::INTERNAL;
    wcs::Wcs wcs;                                     ///< Valid when status is OK; for the whole frame
    std::vector<detection::DetectedStar> stars;       ///< The stars the solve used, frame pixels (blind only)
    SolveMetrics metrics;
    std::string message;                              ///< Why it failed, when it did
};

// ---------------------------------------------------------------------------------------------------------------------
// CLASSES

/// One solve at a time per Solver (it is not thread safe); open one per thread to solve in parallel.
class Solver
{
public:
    /// Opens the star database and, if configured, the quad index (building it the first time).
    static Status open(const SolverConfig& config, std::unique_ptr<Solver>& out,
                       std::string* message = nullptr) noexcept;

    ~Solver();
    Solver(const Solver&) = delete;
    Solver& operator=(const Solver&) = delete;

    SolveResult solve(const common::Frame& frame, const SolveRequest& request) noexcept;

    /// Size of the loaded quad index; 0 without one.
    double indexGigabytes() const noexcept;

private:
    Solver();
    struct Impl;
    std::unique_ptr<Impl> impl_;
};

} // namespace dpot::solve
