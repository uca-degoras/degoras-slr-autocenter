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

/** @file star_detector.h
 *  @brief One pass star detector for plate solving: much faster than faster-astap's find_stars on a binned frame.
 *  @author Degoras Project Team
 *  @copyright GPL-3.0-or-later
 */

#pragma once

// C++ INCLUDES
#include <cstddef>
#include <vector>

// PROJECT INCLUDES
#include "common/frame.h"
#include "detection/background.h"

// NAMESPACES
namespace dpot::detection
{

// ---------------------------------------------------------------------------------------------------------------------
// TYPES

struct DetectedStar
{
    double x = 0;      ///< Frame pixel convention of the image detected on
    double y = 0;
    double flux = 0;   ///< Above the background, 5x5 sum
    double snr = 0;    ///< Background limited: flux / (5 noise)
};

struct DetectorOptions
{
    float sigma = 5;              ///< A peak must stand this many noise sigmas above its cell's background
    double minSeparation = 5;     ///< px between two kept stars
    int maxStars = 200;           ///< The brightest this many are kept
};

// ---------------------------------------------------------------------------------------------------------------------
// FUNCTIONS

/// Local maxima `sigma` above their cell's background with two of the four pixels beside them lit (3 sigma; not a hot
/// pixel), centroided over 5x5, the brightest maxStars kept at least minSeparation px apart. Rows in parallel.
/// `candidates` receives how many maxima passed before the selection.
/// ponytail: no shape test, so halo fragments of a saturated star pass as stars; the quad matching tolerates a few.
std::vector<DetectedStar> detectStars(const common::FloatImage& image, const BackgroundGrid& background,
                                      const DetectorOptions& options, std::size_t* candidates = nullptr);

} // namespace dpot::detection
