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

/** @file fk5.h
 *  @brief The FK5 catalogue (CDS I/149A, 1535 basic stars, and I/175, 3117 extension stars): J2000 positions and
 *         proper motions of the stars the station calibrates its pointing on. Files in data/catalogs/fk5.
 *  @author Degoras Project Team
 *  @copyright GPL-3.0-or-later
 */

#pragma once

// C++ INCLUDES
#include <string>
#include <vector>

// PROJECT INCLUDES
#include "common/angles.h"

// NAMESPACES
namespace dpot::catalog
{

// ---------------------------------------------------------------------------------------------------------------------
// TYPES

struct Fk5Star
{
    int number = 0;      ///< FK5 number
    double ra = 0;       ///< J2000, epoch J2000, rad
    double dec = 0;      ///< rad
    double pmRa = 0;     ///< Proper motion in RA (d alpha / dt, not times cos dec), rad per Julian year
    double pmDec = 0;    ///< rad per Julian year
    double vmag = 0;     ///< Visual magnitude
    int hd = 0;          ///< Henry Draper number, 0 if none: the key to the proper names
};

// ---------------------------------------------------------------------------------------------------------------------
// FUNCTIONS

/// Reads both parts from the catalogs directory (<dir>/fk5/I_149A/catalog and <dir>/fk5/I_175/fk5e.dat). False, with
/// the reason in `error`, when a file is missing.
bool readFk5(const std::string& catalogsDir, std::vector<Fk5Star>& out, std::string* error = nullptr);

/// The star with that FK5 number, or null.
const Fk5Star* findFk5(const std::vector<Fk5Star>& stars, int number);

/// Its position moved by its proper motion to `yearsSinceJ2000` (still in the J2000 frame).
/// ponytail: linear in RA and Dec, exact to well under 0.01" for decades except within a degree of the pole; rigorous
/// space motion (with parallax and radial velocity) when that matters.
common::RaDec positionAt(const Fk5Star& star, double yearsSinceJ2000);

} // namespace dpot::catalog
