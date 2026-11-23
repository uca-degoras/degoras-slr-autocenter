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

/** @file horizon.cpp
 *  @brief Implementation of horizon.h (port of the experiment's altaz_check.py).
 *  @author Degoras Project Team
 *  @copyright GPL-3.0-or-later
 */

// C++ INCLUDES
#include <cmath>

// PROJECT INCLUDES
#include "pointing/horizon.h"

// NAMESPACES
namespace dpot::pointing
{

Site sfelSite()
{
    return {36.465257806 * common::kDegToRad, -6.205305006 * common::kDegToRad, 98.2684};
}

double greenwichMeanSiderealTime(double jd)
{
    const double d = jd - 2451545.0, t = d / 36525;
    return common::wrapTwoPi((280.46061837 + 360.98564736629 * d + 0.000387933 * t * t - t * t * t / 38710000) *
                             common::kDegToRad);
}

common::RaDec precessFromJ2000(const common::RaDec& p, double t)
{
    const double s = common::kArcsecToRad;
    const double zeta = (2306.2181 * t + 0.30188 * t * t + 0.017998 * t * t * t) * s;
    const double z = (2306.2181 * t + 1.09468 * t * t + 0.018203 * t * t * t) * s;
    const double th = (2004.3109 * t - 0.42665 * t * t - 0.041833 * t * t * t) * s;
    const double a = std::cos(p.dec) * std::sin(p.ra + zeta);
    const double b = std::cos(th) * std::cos(p.dec) * std::cos(p.ra + zeta) - std::sin(th) * std::sin(p.dec);
    const double c = std::sin(th) * std::cos(p.dec) * std::cos(p.ra + zeta) + std::cos(th) * std::sin(p.dec);
    return {common::wrapTwoPi(std::atan2(a, b) + z), std::asin(c)};
}

void hourAngleToHorizon(double ha, double dec, double lat, double& az, double& el)
{
    el = std::asin(std::sin(lat) * std::sin(dec) + std::cos(lat) * std::cos(dec) * std::cos(ha));
    az = common::wrapTwoPi(std::atan2(-std::cos(dec) * std::sin(ha),
                                      std::sin(dec) * std::cos(lat) - std::cos(dec) * std::cos(ha) * std::sin(lat)));
}

double parallacticAngle(double ha, double dec, double lat)
{
    return std::atan2(std::sin(ha), std::tan(lat) * std::cos(dec) - std::sin(dec) * std::cos(ha));
}

HorizonPosition toHorizon(const common::RaDec& j2000, const common::UtcTime& utc, const Site& site)
{
    const double jd = common::julianDate(utc), t = (jd - 2451545.0) / 36525;
    // chi: the position angle of date of a point 1" north of the star in J2000.
    const common::RaDec p = precessFromJ2000(j2000, t);
    const common::RaDec n = precessFromJ2000({j2000.ra, j2000.dec + common::kArcsecToRad}, t);
    HorizonPosition h;
    const double dra = n.ra - p.ra;
    h.northAngle = std::atan2(std::cos(n.dec) * std::sin(dra),
                              std::cos(p.dec) * std::sin(n.dec) - std::sin(p.dec) * std::cos(n.dec) * std::cos(dra));
    const double ha = greenwichMeanSiderealTime(jd) + site.longitude - p.ra;
    hourAngleToHorizon(ha, p.dec, site.latitude, h.azimuth, h.elevation);
    h.parallacticAngle = parallacticAngle(ha, p.dec, site.latitude);
    return h;
}

} // namespace dpot::pointing
