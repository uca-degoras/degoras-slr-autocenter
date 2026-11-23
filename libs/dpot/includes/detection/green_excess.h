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

/** @file green_excess.h
 *  @brief A map of where the green pixels are brighter than the red and blue ones predict: the 532 nm laser beam lights
 *         only the green pixels of the SFEL receiver camera, while stars are white and the sky nearly so.
 *  @author Degoras Project Team
 *  @copyright GPL-3.0-or-later
 */

#pragma once

// PROJECT INCLUDES
#include "detection/bayer.h"
#include "detection/field_stop.h"

// NAMESPACES
namespace dpot::detection
{

/// G - k (R + B) / 2, k from the medians inside the field; the star (within `starRadius` of (starX, starY), skipped
/// when starRadius is 0) and everything outside the field set to the field's median; then minus a 61x61 running mean
/// and smoothed 15x15. Planes and result are half size (one value per Bayer cell). `k` receives the factor used. Not
/// yet checked on a frame with the beam: the station's frames with the laser are still to come.
common::FloatImage greenExcess(const ColourPlanes& planes, const FieldMask& mask, double starX, double starY,
                               double starRadius, double& k);

} // namespace dpot::detection
