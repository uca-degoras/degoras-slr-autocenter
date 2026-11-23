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

/** @file star_measure.h
 *  @brief A star measured where a plate solution predicts it, on the full resolution frame: how FK5 stars are
 *         identified and measured without detecting the whole frame.
 *  @author Degoras Project Team
 *  @copyright GPL-3.0-or-later
 */

#pragma once

// PROJECT INCLUDES
#include "common/frame.h"

// NAMESPACES
namespace dpot::measure
{

struct StarMeasurement
{
    double x = 0;        ///< Frame pixels
    double y = 0;
    double snr = 0;      ///< From ASTAP's hfd(); 0 for a blob
    int saturated = 0;   ///< Pixels at 65535 within 50 px
    bool blob = false;   ///< Heavily saturated (more than 100 such pixels): centroid of the pixels above half the peak
};

/// The star near (x, y): its peak is looked for within 8 px of the prediction (3x3 means), and the measurement needs it
/// 10 sigma above the background (median and MAD of the border of a 101 px window). Then ASTAP's hfd() (faster-astap)
/// centroid, recentred once, or for a heavily saturated star the blob centroid, twice. False when no star stands out,
/// or within 80 px of the frame edge.
bool measureStar(const common::Frame& frame, double x, double y, StarMeasurement& out);

} // namespace dpot::measure
