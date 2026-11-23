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

/** @file blobs.h
 *  @brief Connected groups of pixels above a level: the sources in a frame whatever their shape (a star, a satellite
 *         trail, the laser beam), with their flux, centre and elongation.
 *  @author Degoras Project Team
 *  @copyright GPL-3.0-or-later
 */

#pragma once

// C++ INCLUDES
#include <vector>

// PROJECT INCLUDES
#include "common/frame.h"

// NAMESPACES
namespace dpot::detection
{

struct Blob
{
    int pixels = 0;
    double flux = 0;         ///< Sum of (value - background)
    double x = 0;            ///< Flux weighted centre
    double y = 0;
    double elongation = 1;   ///< Major / minor axis of the second moments: ~1 for a star, large for a trail
    double angleDeg = 0;     ///< Direction of the major axis, from +x towards +y (down)
};

/// 4-connected groups of at least `minPixels` pixels at or above `level`, brightest first.
std::vector<Blob> findBlobs(const common::FloatImage& image, float background, float level, int minPixels = 3);

} // namespace dpot::detection
