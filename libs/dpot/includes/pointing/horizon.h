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

/** @file horizon.h
 *  @brief From J2000 RA/Dec to azimuth and elevation at the station.
 *
 *  ACCURACY: the chain of the experiment (docs of the experiment, 11_apuntado_altaz.md): IAU 1976 precession, GMST
 *  (IAU 1982) with UT1 taken as UTC. Left out: nutation (up to ~17"), annual aberration (up to ~20"), refraction (~1'
 *  at 45 deg elevation, more below) and polar motion. Good for the camera geometry (roll, parallactic angle) and for
 *  differences between two nearby points; NOT for absolute pointing models. For those the plan is SuperNOVAS
 *  (apparent and observed place), as decided for DegorasAstrometry.
 *  @author Degoras Project Team
 *  @copyright GPL-3.0-or-later
 */

#pragma once

// PROJECT INCLUDES
#include "common/angles.h"
#include "common/utc_time.h"

// NAMESPACES
namespace dpot::pointing
{

// ---------------------------------------------------------------------------------------------------------------------
// TYPES

struct Site
{
    double latitude = 0;    ///< rad, north positive
    double longitude = 0;   ///< rad, EAST positive
    double height = 0;      ///< m above the ellipsoid
};

struct HorizonPosition
{
    double azimuth = 0;            ///< rad, from north through east, [0, 2 pi)
    double elevation = 0;          ///< rad
    double parallacticAngle = 0;   ///< q: position angle of the zenith at the star, rad
    double northAngle = 0;         ///< chi: position angle (of date) of J2000 north at the star, rad (large near
                                   ///< the pole)
};

// ---------------------------------------------------------------------------------------------------------------------
// FUNCTIONS

/// The SFEL laser ranging station (San Fernando, IVP of the ILRS site log 2025): 36.465257806 N, 6.205305006 W,
/// 98.2684 m.
Site sfelSite();

/// Greenwich mean sidereal time (IAU 1982), rad.
double greenwichMeanSiderealTime(double julianDateUt1);

/// J2000 mean place precessed to the mean equator and equinox of date (IAU 1976), `centuries` since J2000.
common::RaDec precessFromJ2000(const common::RaDec& j2000, double centuries);

/// Hour angle and declination -> azimuth (from north through east) and elevation, at `latitude`. All rad.
void hourAngleToHorizon(double hourAngle, double dec, double latitude, double& azimuth, double& elevation);

/// Parallactic angle q (position angle of the zenith) for that hour angle and declination, rad.
double parallacticAngle(double hourAngle, double dec, double latitude);

/// The whole chain for a J2000 position (proper motion already applied) at a UTC instant.
HorizonPosition toHorizon(const common::RaDec& j2000, const common::UtcTime& utc, const Site& site);

} // namespace dpot::pointing
