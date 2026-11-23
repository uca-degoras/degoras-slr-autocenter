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

/** @file fk5.cpp
 *  @brief Implementation of fk5.h.
 *  @author Degoras Project Team
 *  @copyright GPL-3.0-or-later
 */

// C++ INCLUDES
#include <cstdlib>
#include <fstream>

// PROJECT INCLUDES
#include "catalog/fk5.h"

// NAMESPACES
namespace dpot::catalog
{

namespace
{

// One file. I/149A and I/175 share the columns read here (their ReadMe files, 1-based byte ranges): number 1-4,
// RA 6-17 (h m s), pm RA 19-25 (s of time per century), Dec sign 27, Dec 28-38 (d m s), pm Dec 40-46 ("/century),
// V magnitude 124-128, HD number 167-172.
bool readOne(const std::string& file, std::vector<Fk5Star>& out, std::string* error)
{
    std::ifstream in(file);
    if (!in)
    {
        if (error)
            *error = "cannot open " + file;
        return false;
    }
    for (std::string l; std::getline(in, l);)
    {
        if (l.size() < 128)
            continue;
        auto num = [&l](std::size_t from, std::size_t to) {
            return from - 1 < l.size() ? std::atof(l.substr(from - 1, to - from + 1).c_str()) : 0.0;
        };
        const double sign = l[26] == '-' ? -1 : 1;
        Fk5Star s;
        s.number = static_cast<int>(num(1, 4));
        s.ra = (num(6, 7) + num(9, 10) / 60 + num(12, 17) / 3600) * 15 * common::kDegToRad;
        s.dec = sign * (num(28, 29) + num(31, 32) / 60 + num(34, 38) / 3600) * common::kDegToRad;
        s.pmRa = num(19, 25) * 15 * common::kArcsecToRad / 100;
        s.pmDec = num(40, 46) * common::kArcsecToRad / 100;
        s.vmag = num(124, 128);
        s.hd = static_cast<int>(num(167, 172));
        out.push_back(s);
    }
    return true;
}

} // namespace

bool readFk5(const std::string& catalogsDir, std::vector<Fk5Star>& out, std::string* error)
{
    return readOne(catalogsDir + "/fk5/I_149A/catalog", out, error) &&
           readOne(catalogsDir + "/fk5/I_175/fk5e.dat", out, error);
}

const Fk5Star* findFk5(const std::vector<Fk5Star>& stars, int number)
{
    for (const Fk5Star& s : stars)
        if (s.number == number)
            return &s;
    return nullptr;
}

common::RaDec positionAt(const Fk5Star& star, double yearsSinceJ2000)
{
    return {star.ra + star.pmRa * yearsSinceJ2000, star.dec + star.pmDec * yearsSinceJ2000};
}

} // namespace dpot::catalog
