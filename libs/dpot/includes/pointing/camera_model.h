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

/** @file camera_model.h
 *  @brief How a camera fixed to the tube of the alt-az mount sees the sky: scale, roll and the pixel the telescope axis
 *         falls on. From there, where a star is relative to the axis in the mount's own axes, and the pair a pointing
 *         model (TPOINT) is fitted from.
 *  @author Degoras Project Team
 *  @copyright GPL-3.0-or-later
 */

#pragma once

// PROJECT INCLUDES
#include "pointing/horizon.h"

// NAMESPACES
namespace dpot::pointing
{

// ---------------------------------------------------------------------------------------------------------------------
// TYPES

/// With roll 0 and no mirror, frame x grows with azimuth and frame y DOWN means lower elevation (image up = +El).
struct CameraModel
{
    double arcsecPerPixel = 1;
    double rollDeg = 0;      ///< The image's +Az axis turned this much counter-clockwise (as seen on the image)
    bool mirrored = false;   ///< x grows against azimuth
    double axisX = 0;        ///< Frame pixel where the telescope axis falls
    double axisY = 0;
};

/// Offsets in the mount's axes, arcsec.
struct MountOffset
{
    double dAzCosEl = 0;   ///< Azimuth difference times cos(elevation): a true angle on the sky
    double dEl = 0;
};

struct EncoderReading
{
    double azimuth = 0;     ///< rad
    double elevation = 0;   ///< rad
};

/// One observation for a pointing model: where the star is (demanded, computed) and where the encoders would read
/// with the star on the axis (observed). Radians.
struct TpointPair
{
    double demandedAz = 0, demandedEl = 0;
    double observedAz = 0, observedEl = 0;
};

// ---------------------------------------------------------------------------------------------------------------------
// FUNCTIONS

/// The SFEL receiver camera (ASI585MC Pro, raw pixels, 3840x2160), calibrated on the 2026-10-28 frames against Gaia
/// DR3 (experiment, 12_calibracion.md): 0.0758"/px, roll +1.0 deg, normal parity. Axis: the image centre; the field
/// stop centre (dpot_spot prints it) is the better estimate where it can be fitted.
CameraModel sfelReceiverCamera();

/// The RASA 8 camera (ASI2600MM Pro, 6248x4176): 1.932"/px from the plate solutions, roll -0.76 deg (four stars,
/// 11_apuntado_altaz.md; -0.6 .. -0.8 depending on elevation). Axis: the image centre.
CameraModel rasaCamera();

/// A frame displacement (dx, dy in frame pixels, y down) in the mount's axes.
MountOffset frameToMount(const CameraModel& camera, double dx, double dy);

/// Where the star at frame pixel (x, y) is relative to the telescope axis: star minus axis.
MountOffset starOffset(const CameraModel& camera, double x, double y);

/// The pair for a pointing model, from the star's computed position, the encoders at the frame's time and the star's
/// offset from the axis (starOffset). See horizon.h about the accuracy of the computed position.
TpointPair tpointPair(const HorizonPosition& star, const EncoderReading& encoders, const MountOffset& starMinusAxis);

} // namespace dpot::pointing
