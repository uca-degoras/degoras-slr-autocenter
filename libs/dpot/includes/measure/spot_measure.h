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

/** @file spot_measure.h
 *  @brief The one bright star of a high magnification frame (the SFEL receiver camera: 0.076"/px, a single star, no
 *         field to solve), measured twice: centre of gravity and Gaussian-windowed centroid.
 *  @author Degoras Project Team
 *  @copyright GPL-3.0-or-later
 */

#pragma once

// C++ INCLUDES
#include <vector>

// PROJECT INCLUDES
#include "common/frame.h"
#include "detection/blobs.h"

// NAMESPACES
namespace dpot::measure
{

struct SpotOptions
{
    int seedX = -1;   ///< With a seed, the brightest star within `search` px of it; without, the brightest one
    int seedY = -1;
    int search = 0;
    int rcog = 50;    ///< Radius of the centre of gravity, px (50 for the bright star; less for a faint neighbour)
};

struct SpotMeasurement
{
    double winX = 0, winY = 0;   ///< Windowed centroid (SExtractor's XWIN): the precise one
    double cogX = 0, cogY = 0;   ///< Centre of gravity above 3 sigma, recentred once (like ASTAP's hfd())
    double sigma = 0;            ///< From the second moments; FWHM = 2.3548 sigma
    double flux = 0;
    float background = 0, noise = 0, peak = 0;
    int seedX = 0, seedY = 0;
    std::vector<detection::Blob> sources;   ///< Without a seed: everything 10 sigma out, brightest first

    bool found() const;   ///< Finite centre and a peak 10 sigma above the background
};

/// On a grey image (bayerSum of a raw frame, or any grey frame), image pixels. Background and noise: median and MAD of
/// a ring of radius rcog+10 .. rcog+30 around the brightest 5x5 sum.
SpotMeasurement measureSpot(const common::FloatImage& grey, const SpotOptions& options = {});

/// Pixels of `raw` at the top of its range (255 for RAW8) in the square of 2*halfSide raw pixels around (x, y).
int countSaturated(const common::Frame& raw, double x, double y, int halfSide);

} // namespace dpot::measure
