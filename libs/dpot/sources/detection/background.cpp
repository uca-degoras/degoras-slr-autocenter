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

/** @file background.cpp
 *  @brief Implementation of background.h.
 *  @author Degoras Project Team
 *  @copyright GPL-3.0-or-later
 */

// C++ INCLUDES
#include <algorithm>
#include <utility>

// PROJECT INCLUDES
#include "common/parallel.h"
#include "common/stats.h"
#include "detection/background.h"

// NAMESPACES
namespace dpot::detection
{

BackgroundGrid::BackgroundGrid(const common::FloatImage& image) :
    nx((image.width + kCell - 1) / kCell),
    ny((image.height + kCell - 1) / kCell),
    level(static_cast<std::size_t>(nx) * static_cast<std::size_t>(ny)),
    noise(level.size())
{
    common::parallelFor(0, level.size(), [&](std::size_t c) {
        const int x0 = static_cast<int>(c % static_cast<std::size_t>(nx)) * kCell;
        const int y0 = static_cast<int>(c / static_cast<std::size_t>(nx)) * kCell;
        std::vector<float> v;
        for (int y = y0; y < std::min(image.height, y0 + kCell); y += kStride)
            for (int x = x0; x < std::min(image.width, x0 + kCell); x += kStride)
                v.push_back(image.at(x, y));
        level[c] = common::median(v);
        noise[c] = std::max(common::madSigma(std::move(v), level[c]), 1.0f);
    });
}

common::FloatImage boxMean(const common::FloatImage& image, int r)
{
    const int w = image.width, h = image.height;
    const std::size_t sw = static_cast<std::size_t>(w) + 1;
    std::vector<double> s(sw * (static_cast<std::size_t>(h) + 1), 0);
    for (int y = 0; y < h; y++)
        for (int x = 0; x < w; x++)
        {
            const std::size_t yy = static_cast<std::size_t>(y), xx = static_cast<std::size_t>(x);
            s[(yy + 1) * sw + xx + 1] = image.at(x, y) + s[yy * sw + xx + 1] + s[(yy + 1) * sw + xx] - s[yy * sw + xx];
        }
    common::FloatImage out(w, h);
    for (int y = 0; y < h; y++)
        for (int x = 0; x < w; x++)
        {
            const auto x0 = static_cast<std::size_t>(std::max(0, x - r));
            const auto x1 = static_cast<std::size_t>(std::min(w, x + r + 1));
            const auto y0 = static_cast<std::size_t>(std::max(0, y - r));
            const auto y1 = static_cast<std::size_t>(std::min(h, y + r + 1));
            const double sum = s[y1 * sw + x1] - s[y0 * sw + x1] - s[y1 * sw + x0] + s[y0 * sw + x0];
            out.at(x, y) = static_cast<float>(sum / static_cast<double>((x1 - x0) * (y1 - y0)));
        }
    return out;
}

} // namespace dpot::detection
