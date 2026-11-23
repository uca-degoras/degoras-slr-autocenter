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

/** @file star_names.cpp
 *  @brief Implementation of star_names.h.
 *  @author Degoras Project Team
 *  @copyright GPL-3.0-or-later
 */

// C++ INCLUDES
#include <algorithm>
#include <cstdlib>
#include <fstream>
#include <sstream>
#include <vector>

// PROJECT INCLUDES
#include "catalog/star_names.h"

// NAMESPACES
namespace dpot::catalog
{

std::unordered_map<int, std::string> readIauNames(const std::string& file)
{
    // The ASCII name fills the first 18 columns; the rest are space separated, but the ones before HD hold spaces and
    // UTF-8, so HD is found as the third token before the date (YYYY-MM-DD).
    std::unordered_map<int, std::string> out;
    std::ifstream in(file);
    for (std::string l; std::getline(in, l);)
    {
        if (l.size() < 18 || l[0] == '#' || l[0] == '$')
            continue;
        std::istringstream ss(l.substr(18));
        std::vector<std::string> tok;
        for (std::string t; ss >> t;)
            tok.push_back(t);
        const auto date = std::find_if(tok.begin(), tok.end(), [](const std::string& t) {
            return t.size() == 10 && t[4] == '-' && t[7] == '-';
        });
        if (date - tok.begin() < 3)
            continue;
        std::string name = l.substr(0, 18);
        name.erase(name.find_last_not_of(' ') + 1);
        if (const int hd = std::atoi((date - 3)->c_str()); hd > 0)
            out.emplace(hd, name);
    }
    return out;
}

std::string fk5Label(const Fk5Star& star, const std::unordered_map<int, std::string>& names)
{
    const auto n = names.find(star.hd);
    return "FK5 " + std::to_string(star.number) + (n == names.end() ? "" : "\n" + n->second);
}

} // namespace dpot::catalog
