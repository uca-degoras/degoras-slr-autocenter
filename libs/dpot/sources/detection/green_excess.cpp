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

/** @file green_excess.cpp
 *  @brief Implementation of green_excess.h.
 *  @author Degoras Project Team
 *  @copyright GPL-3.0-or-later
 */

// C++ INCLUDES
#include <algorithm>
#include <cmath>
#include <vector>

// PROJECT INCLUDES
#include "common/stats.h"
#include "detection/background.h"
#include "detection/green_excess.h"

// NAMESPACES
namespace dpot::detection
{

common::FloatImage greenExcess(const ColourPlanes& planes, const FieldMask& mask, double starX, double starY,
                               double starRadius, double& k)
{
    const common::FloatImage &r = planes.r, &g = planes.g, &b = planes.b;
    std::vector<float> sg, srb;
    for (std::size_t i = 0; i < g.px.size(); i += 5)
        if (mask.field[i])
        {
            sg.push_back(g.px[i]);
            srb.push_back((r.px[i] + b.px[i]) / 2);
        }
    k = common::median(sg) / std::max(0.01f, common::median(srb));
    common::FloatImage e(g.width, g.height);
    for (std::size_t i = 0; i < e.px.size(); i++)
        e.px[i] = static_cast<float>(g.px[i] - k * (r.px[i] + b.px[i]) / 2);
    std::vector<float> s;
    for (std::size_t i = 0; i < e.px.size(); i += 5)
        if (mask.field[i])
            s.push_back(e.px[i]);
    const float med = common::median(s);
    for (int y = 0; y < e.height; y++)
        for (int x = 0; x < e.width; x++)
        {
            const auto i = static_cast<std::size_t>(y) * static_cast<std::size_t>(e.width) +
                           static_cast<std::size_t>(x);
            if (!mask.field[i] || (starRadius > 0 && std::hypot(x - starX, y - starY) < starRadius))
                e.px[i] = med;
        }
    const common::FloatImage low = boxMean(e, 30);
    for (std::size_t i = 0; i < e.px.size(); i++)
        e.px[i] -= low.px[i];
    return boxMean(e, 7);
}

} // namespace dpot::detection
