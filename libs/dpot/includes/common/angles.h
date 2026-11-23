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

/** @file angles.h
 *  @brief Angle constants, sky coordinates and wrapping. Angles are radians unless a name says otherwise.
 *  @author Degoras Project Team
 *  @copyright GPL-3.0-or-later
 */

#pragma once

// C++ INCLUDES
#include <cmath>

// NAMESPACES
namespace dpot::common
{

// ---------------------------------------------------------------------------------------------------------------------
// CONSTANTS

constexpr double kPi = 3.14159265358979323846;
constexpr double kDegToRad = kPi / 180.0;
constexpr double kRadToDeg = 180.0 / kPi;
constexpr double kArcsecToRad = kDegToRad / 3600.0;
constexpr double kRadToArcsec = 1.0 / kArcsecToRad;

// ---------------------------------------------------------------------------------------------------------------------
// TYPES

/// Right ascension and declination, radians. J2000 (FK5/ICRS) unless the function that returns it says otherwise.
struct RaDec
{
    double ra = 0;
    double dec = 0;
};

// ---------------------------------------------------------------------------------------------------------------------
// FUNCTIONS

/// The angle brought into [-pi, pi).
inline double wrapPi(double a)
{
    a = std::fmod(a + kPi, 2 * kPi);
    return (a < 0 ? a + 2 * kPi : a) - kPi;
}

/// The angle brought into [0, 2 pi).
inline double wrapTwoPi(double a)
{
    a = std::fmod(a, 2 * kPi);
    return a < 0 ? a + 2 * kPi : a;
}

} // namespace dpot::common
