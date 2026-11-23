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

/** @file blobs.cpp
 *  @brief Implementation of blobs.h.
 *  @author Degoras Project Team
 *  @copyright GPL-3.0-or-later
 */

// C++ INCLUDES
#include <initializer_list>
#include <algorithm>
#include <cmath>

// PROJECT INCLUDES
#include "common/angles.h"
#include "detection/blobs.h"

// NAMESPACES
namespace dpot::detection
{

std::vector<Blob> findBlobs(const common::FloatImage& image, float background, float level, int minPixels)
{
    const int w = image.width;
    const int n = static_cast<int>(image.px.size());
    std::vector<char> seen(image.px.size(), 0);
    std::vector<Blob> out;
    std::vector<int> stack;
    for (int i = 0; i < n; i++)
    {
        if (seen[static_cast<std::size_t>(i)] || image.px[static_cast<std::size_t>(i)] < level)
            continue;
        int count = 0;
        double s = 0, sx = 0, sy = 0, sxx = 0, syy = 0, sxy = 0;
        stack.assign(1, i);
        seen[static_cast<std::size_t>(i)] = 1;
        while (!stack.empty())
        {
            const int k = stack.back();
            stack.pop_back();
            const int x = k % w, y = k / w;
            const double f = image.px[static_cast<std::size_t>(k)] - background;
            count++;
            s += f;
            sx += f * x;
            sy += f * y;
            sxx += f * x * x;
            syy += f * y * y;
            sxy += f * x * y;
            for (const int nb : {x > 0 ? k - 1 : -1, x + 1 < w ? k + 1 : -1, k - w, k + w < n ? k + w : -1})
                if (nb >= 0 && !seen[static_cast<std::size_t>(nb)] && image.px[static_cast<std::size_t>(nb)] >= level)
                {
                    seen[static_cast<std::size_t>(nb)] = 1;
                    stack.push_back(nb);
                }
        }
        if (count < minPixels || s <= 0)
            continue;
        Blob b;
        b.pixels = count;
        b.flux = s;
        b.x = sx / s;
        b.y = sy / s;
        const double a = sxx / s - b.x * b.x, c = syy / s - b.y * b.y, cxy = sxy / s - b.x * b.y;
        const double tr = (a + c) / 2, d = std::sqrt(std::max(0.0, tr * tr - (a * c - cxy * cxy)));
        b.elongation = tr - d > 1e-12 ? std::sqrt((tr + d) / (tr - d)) : 1;
        b.angleDeg = 0.5 * std::atan2(2 * cxy, a - c) * common::kRadToDeg;
        out.push_back(b);
    }
    std::sort(out.begin(), out.end(), [](const Blob& a, const Blob& b) { return a.flux > b.flux; });
    return out;
}

} // namespace dpot::detection
