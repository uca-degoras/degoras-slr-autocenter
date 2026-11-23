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

/** @file star_detector.cpp
 *  @brief Implementation of star_detector.h.
 *  @author Degoras Project Team
 *  @copyright GPL-3.0-or-later
 */

// C++ INCLUDES
#include <algorithm>
#include <cmath>

// PROJECT INCLUDES
#include "common/parallel.h"
#include "detection/star_detector.h"

// NAMESPACES
namespace dpot::detection
{

std::vector<DetectedStar> detectStars(const common::FloatImage& image, const BackgroundGrid& background,
                                      const DetectorOptions& options, std::size_t* candidates)
{
    const int w = image.width, h = image.height;
    const std::size_t chunks = 4 * static_cast<std::size_t>(common::threadCount());
    std::vector<std::vector<DetectedStar>> found(chunks);
    common::parallelFor(0, chunks, [&](std::size_t k) {
        const int ya = std::max(3, static_cast<int>(static_cast<std::size_t>(h) * k / chunks));
        const int yb = std::min(h - 3, static_cast<int>(static_cast<std::size_t>(h) * (k + 1) / chunks));
        for (int y = ya; y < yb; y++)
        {
            const float *r = image.row(y), *p = image.row(y - 1), *n = image.row(y + 1);
            for (int x = 3; x < w - 3; x++)
            {
                const std::size_t c = background.cellOf(x, y);
                const float v = r[x], bg = background.level[c], sd = background.noise[c];
                if (v < bg + options.sigma * sd)
                    continue;
                // The maximum of its 3x3; on a flat (saturated) top, the first pixel in scan order.
                if (!(v > r[x - 1] && v > p[x - 1] && v > p[x] && v > p[x + 1] && v >= r[x + 1] && v >= n[x - 1] &&
                      v >= n[x] && v >= n[x + 1]))
                    continue;
                const float lit = bg + 3 * sd;
                if ((r[x - 1] > lit) + (r[x + 1] > lit) + (p[x] > lit) + (n[x] > lit) < 2)
                    continue;
                double s = 0, sx = 0, sy = 0;
                for (int j = -2; j <= 2; j++)
                {
                    const float* rr = image.row(y + j);
                    for (int i = -2; i <= 2; i++)
                    {
                        const double u = rr[x + i] - bg;
                        if (u <= 0)
                            continue;
                        s += u;
                        sx += u * i;
                        sy += u * j;
                    }
                }
                found[k].push_back({x + sx / s, y + sy / s, s, s / (5 * sd)});
            }
        }
    });
    std::vector<DetectedStar> all;
    for (const auto& f : found)
        all.insert(all.end(), f.begin(), f.end());
    if (candidates)
        *candidates = all.size();
    std::sort(all.begin(), all.end(), [](const DetectedStar& a, const DetectedStar& b) { return a.flux > b.flux; });
    std::vector<DetectedStar> kept;
    for (const DetectedStar& c : all)
    {
        if (kept.size() >= static_cast<std::size_t>(options.maxStars))
            break;
        const bool apart = std::all_of(kept.begin(), kept.end(), [&](const DetectedStar& k) {
            return std::hypot(c.x - k.x, c.y - k.y) >= options.minSeparation;
        });
        if (apart)
            kept.push_back(c);
    }
    return kept;
}

} // namespace dpot::detection
