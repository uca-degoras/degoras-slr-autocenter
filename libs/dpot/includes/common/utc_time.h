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

/** @file utc_time.h
 *  @brief A UTC instant and the Julian dates and epochs derived from it.
 *  @author Degoras Project Team
 *  @copyright GPL-3.0-or-later
 */

#pragma once

// C++ INCLUDES
#include <string>

// NAMESPACES
namespace dpot::common
{

// ---------------------------------------------------------------------------------------------------------------------
// TYPES

/// A UTC instant, Gregorian calendar.
struct UtcTime
{
    int year = 2000;
    int month = 1;
    int day = 1;
    int hour = 12;
    int minute = 0;
    double second = 0;
};

// ---------------------------------------------------------------------------------------------------------------------
// FUNCTIONS

/// Julian date (Meeus, Astronomical Algorithms, ch. 7), UTC taken as UT1 (|UT1 - UTC| < 0.9 s).
double julianDate(const UtcTime& t);

/// Julian years since J2000.0 (JD 2451545.0): the epoch proper motions are applied to.
double julianYearsSinceJ2000(const UtcTime& t);

/// Julian centuries since J2000.0: the time argument of precession and sidereal time.
double julianCenturiesSinceJ2000(const UtcTime& t);

/// "2026-10-28T19:24:41.250Z".
std::string toIso(const UtcTime& t);

/// Reads "YYYY-MM-DDTHH:MM:SS[.fff][Z]" (also with a space instead of T). False if it does not parse.
bool parseIso(const std::string& text, UtcTime& out);

} // namespace dpot::common
