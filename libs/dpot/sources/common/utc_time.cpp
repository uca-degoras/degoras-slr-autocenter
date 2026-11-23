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

/** @file utc_time.cpp
 *  @brief Implementation of utc_time.h.
 *  @author Degoras Project Team
 *  @copyright GPL-3.0-or-later
 */

// C++ INCLUDES
#include <cstdio>

// PROJECT INCLUDES
#include "common/utc_time.h"

// NAMESPACES
namespace dpot::common
{

double julianDate(const UtcTime& t)
{
    // Julian day number of the civil date (valid for any Gregorian date after -4800), then the time of day.
    const int a = (14 - t.month) / 12;
    const long y = t.year + 4800 - a, m = t.month + 12 * a - 3;
    const long jdn = t.day + (153 * m + 2) / 5 + 365 * y + y / 4 - y / 100 + y / 400 - 32045;
    const double hours = t.hour + t.minute / 60.0 + t.second / 3600.0;
    return static_cast<double>(jdn) + (hours - 12) / 24;
}

double julianYearsSinceJ2000(const UtcTime& t)
{
    return (julianDate(t) - 2451545.0) / 365.25;
}

double julianCenturiesSinceJ2000(const UtcTime& t)
{
    return (julianDate(t) - 2451545.0) / 36525.0;
}

std::string toIso(const UtcTime& t)
{
    char text[40];
    std::snprintf(text, sizeof text, "%04d-%02d-%02dT%02d:%02d:%06.3fZ", t.year, t.month, t.day, t.hour, t.minute,
                  t.second);
    return text;
}

bool parseIso(const std::string& text, UtcTime& out)
{
    UtcTime t;
    char sep = 0;
    if (std::sscanf(text.c_str(), "%d-%d-%d%c%d:%d:%lf", &t.year, &t.month, &t.day, &sep, &t.hour, &t.minute,
                    &t.second) != 7 || (sep != 'T' && sep != ' '))
        return false;
    if (t.month < 1 || t.month > 12 || t.day < 1 || t.day > 31 || t.hour < 0 || t.hour > 23 || t.minute < 0 ||
        t.minute > 59 || t.second < 0 || t.second >= 61)
        return false;
    out = t;
    return true;
}

} // namespace dpot::common
