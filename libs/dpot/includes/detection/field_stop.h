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

/** @file field_stop.h
 *  @brief The receiver's field stop on a SFEL frame: the lit circle the telescope's optics let through, and its centre,
 *         which is where the telescope axis falls on the camera.
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

struct FieldMask
{
    std::vector<char> field;   ///< Lit pixels: above half way between the dark corners and the middle of the image
    std::vector<char> inner;   ///< At least 45 px inside the field, clear of the ringing of a 61x61 high-pass filter
};

/// The field stop on a mean plane (the mean green plane of a run is the cleanest): dark corners (40x40 each) against
/// the middle (200x200). Needs an image larger than 200x200.
FieldMask fieldMask(const common::FloatImage& plane);

/// Circle through the field's edge by algebraic least squares (x^2 + y^2 + D x + E y + F = 0), in the plane's pixels.
/// Edge pixels on the image border are left out: there the sensor cuts the field, not the stop. False if degenerate.
bool fitFieldStop(const FieldMask& mask, int width, int height, double& cx, double& cy, double& radius);

} // namespace dpot::detection
