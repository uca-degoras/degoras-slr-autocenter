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

/** @file bayer.cpp
 *  @brief Implementation of bayer.h.
 *  @author Degoras Project Team
 *  @copyright GPL-3.0-or-later
 */

// PROJECT INCLUDES
#include "detection/bayer.h"

// NAMESPACES
namespace dpot::detection
{

common::FloatImage bayerSum(const common::Frame& raw)
{
    common::FloatImage g(raw.width / 2, raw.height / 2);
    for (int y = 0; y < g.height; y++)
        for (int x = 0; x < g.width; x++)
            g.at(x, y) = static_cast<float>(raw.raw(2 * x, 2 * y) + raw.raw(2 * x + 1, 2 * y) +
                                            raw.raw(2 * x, 2 * y + 1) + raw.raw(2 * x + 1, 2 * y + 1));
    return g;
}

ColourPlanes splitRggb(const common::Frame& raw)
{
    const int w = raw.width / 2, h = raw.height / 2;
    ColourPlanes p{common::FloatImage(w, h), common::FloatImage(w, h), common::FloatImage(w, h)};
    for (int y = 0; y < h; y++)
        for (int x = 0; x < w; x++)
        {
            p.r.at(x, y) = static_cast<float>(raw.raw(2 * x, 2 * y));
            p.g.at(x, y) = static_cast<float>(raw.raw(2 * x + 1, 2 * y) + raw.raw(2 * x, 2 * y + 1)) / 2;
            p.b.at(x, y) = static_cast<float>(raw.raw(2 * x + 1, 2 * y + 1));
        }
    return p;
}

} // namespace dpot::detection
