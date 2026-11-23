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

/** @file binning.h
 *  @brief Software binning of a frame (and a central crop), the first step of the fast solve: a 3x3 binned RASA frame
 *         has 9 times fewer pixels and its stars are still 2-3 px wide.
 *  @author Degoras Project Team
 *  @copyright GPL-3.0-or-later
 */

#pragma once

// PROJECT INCLUDES
#include "common/frame.h"

// NAMESPACES
namespace dpot::detection
{

/// Where a binned image sits in its frame.
struct BinnedGeometry
{
    int bin = 1;
    int x0 = 0;   ///< Frame pixel of the crop's left column
    int y0 = 0;   ///< Frame pixel of the crop's top row

    /// Binned pixel (u, v) -> frame pixel: the centre of the bin x bin block.
    double frameX(double u) const { return u * bin + (bin - 1) / 2.0 + x0; }
    double frameY(double v) const { return v * bin + (bin - 1) / 2.0 + y0; }
};

/// Mean of bin x bin blocks over the central `crop` fraction of each side (1 = whole frame), rows in parallel.
common::FloatImage binFrame(const common::Frame& frame, int bin, double crop, BinnedGeometry& geometry);

} // namespace dpot::detection
