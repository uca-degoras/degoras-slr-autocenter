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

/** @file star_names.h
 *  @brief IAU proper star names (IAU-CSN, the WGSN list, data/catalogs/names/IAU-CSN.txt), joined to FK5 by HD number.
 *  @author Degoras Project Team
 *  @copyright GPL-3.0-or-later
 */

#pragma once

// C++ INCLUDES
#include <string>
#include <unordered_map>

// PROJECT INCLUDES
#include "catalog/fk5.h"

// NAMESPACES
namespace dpot::catalog
{

/// HD number -> proper name ("Altair"). Empty when the file cannot be read.
std::unordered_map<int, std::string> readIauNames(const std::string& file);

/// "FK5 745", and the proper name on a second line when the star has one ("FK5 745\nAltair").
std::string fk5Label(const Fk5Star& star, const std::unordered_map<int, std::string>& names);

} // namespace dpot::catalog
