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

/** @file bayer.h
 *  @brief Raw Bayer frames (the SFEL receiver camera, ASI585MC Pro, RGGB, saved as RAW8 grey PNGs): a grey image that
 *         the colour pattern does not pull, and the colour planes.
 *  @author Degoras Project Team
 *  @copyright GPL-3.0-or-later
 */

#pragma once

// PROJECT INCLUDES
#include "common/frame.h"

// NAMESPACES
namespace dpot::detection
{

/// Each 2x2 Bayer cell summed into one grey pixel, from the samples as the camera gave them (0..255 for RAW8). Half
/// the frame's width and height.
common::FloatImage bayerSum(const common::Frame& raw);

/// The R, G (mean of the two) and B planes of an RGGB frame, each half size, raw sample values.
struct ColourPlanes
{
    common::FloatImage r, g, b;
};
ColourPlanes splitRggb(const common::Frame& raw);

/// Grey (half size) pixel -> raw frame pixel: grey pixel gx covers raw 2gx..2gx+1, so its centre is raw 2gx + 0.5.
inline double greyToRaw(double v) { return 2 * v + 0.5; }
inline double rawToGrey(double v) { return (v - 0.5) / 2; }

} // namespace dpot::detection
