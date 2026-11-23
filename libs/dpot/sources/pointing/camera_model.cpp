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

/** @file camera_model.cpp
 *  @brief Implementation of camera_model.h.
 *  @author Degoras Project Team
 *  @copyright GPL-3.0-or-later
 */

// C++ INCLUDES
#include <cmath>

// PROJECT INCLUDES
#include "pointing/camera_model.h"

// NAMESPACES
namespace dpot::pointing
{

CameraModel sfelReceiverCamera()
{
    return {0.0758, 1.0, false, (3840 - 1) / 2.0, (2160 - 1) / 2.0};
}

CameraModel rasaCamera()
{
    return {1.932, -0.76, false, (6248 - 1) / 2.0, (4176 - 1) / 2.0};
}

MountOffset frameToMount(const CameraModel& c, double dx, double dy)
{
    const double roll = c.rollDeg * common::kDegToRad;
    const double ux = (c.mirrored ? -dx : dx) * c.arcsecPerPixel;   // along the image's +Az axis
    const double uy = -dy * c.arcsecPerPixel;                        // up the image: +El
    return {ux * std::cos(roll) + uy * std::sin(roll), -ux * std::sin(roll) + uy * std::cos(roll)};
}

MountOffset starOffset(const CameraModel& c, double x, double y)
{
    return frameToMount(c, x - c.axisX, y - c.axisY);
}

TpointPair tpointPair(const HorizonPosition& star, const EncoderReading& enc, const MountOffset& d)
{
    // The axis points at the encoder position; the star is d away from it, so the encoders would read enc + d with the
    // star on the axis.
    TpointPair p;
    p.demandedAz = star.azimuth;
    p.demandedEl = star.elevation;
    p.observedAz = common::wrapTwoPi(enc.azimuth + d.dAzCosEl * common::kArcsecToRad / std::cos(enc.elevation));
    p.observedEl = enc.elevation + d.dEl * common::kArcsecToRad;
    return p;
}

} // namespace dpot::pointing
