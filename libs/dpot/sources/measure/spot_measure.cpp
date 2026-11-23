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

/** @file spot_measure.cpp
 *  @brief Implementation of spot_measure.h.
 *  @author Degoras Project Team
 *  @copyright GPL-3.0-or-later
 */

// C++ INCLUDES
#include <algorithm>
#include <cmath>
#include <utility>

// PROJECT INCLUDES
#include "common/stats.h"
#include "measure/spot_measure.h"

// NAMESPACES
namespace dpot::measure
{

bool SpotMeasurement::found() const
{
    return std::isfinite(winX) && std::isfinite(winY) && peak > background + 10 * noise;
}

SpotMeasurement measureSpot(const common::FloatImage& g, const SpotOptions& o)
{
    SpotMeasurement m;
    const bool seeded = o.seedX >= 0;
    if (!seeded)
    {
        // Global background and noise from every 3rd pixel: the star covers a tiny part of the frame.
        std::vector<float> sample;
        for (int y = 0; y < g.height; y += 3)
            for (int x = 0; x < g.width; x += 3)
                sample.push_back(g.at(x, y));
        const float bg = common::median(sample);
        const float noise = std::max(common::madSigma(std::move(sample), bg), 0.5f);
        m.sources = detection::findBlobs(g, bg, bg + 10 * noise);
    }

    // Brightest 5x5 sum, over the image or around the seed.
    double best = -1;
    const int ya = seeded ? std::max(2, o.seedY - o.search) : 2;
    const int yb = seeded ? std::min(g.height - 3, o.seedY + o.search) : g.height - 3;
    const int xa = seeded ? std::max(2, o.seedX - o.search) : 2;
    const int xb = seeded ? std::min(g.width - 3, o.seedX + o.search) : g.width - 3;
    for (int y = ya; y <= yb; y++)
        for (int x = xa; x <= xb; x++)
        {
            double s = 0;
            for (int j = -2; j <= 2; j++)
                for (int i = -2; i <= 2; i++)
                    s += g.at(x + i, y + j);
            if (s > best)
            {
                best = s;
                m.seedX = x;
                m.seedY = y;
            }
        }

    // Local background and noise from a ring of radius rcog+10 .. rcog+30, inside the image.
    std::vector<float> ring;
    const int r0 = o.rcog + 10, r1 = o.rcog + 30;
    for (int j = -r1; j <= r1; j++)
        for (int i = -r1; i <= r1; i++)
        {
            const int r2 = i * i + j * j, x = m.seedX + i, y = m.seedY + j;
            if (r2 >= r0 * r0 && r2 <= r1 * r1 && x >= 0 && y >= 0 && x < g.width && y < g.height)
                ring.push_back(g.at(x, y));
        }
    m.background = common::median(ring);
    m.noise = std::max(common::madSigma(std::move(ring), m.background), 0.5f);

    // Centre of gravity above 3 sigma within rcog, recentred once; its second moments give the window.
    double cx = m.seedX, cy = m.seedY, mxx = 0, myy = 0;
    for (int pass = 0; pass < 2; pass++)
    {
        double sw = 0, sx = 0, sy = 0, sxx = 0, syy = 0;
        const int x0 = static_cast<int>(std::lround(cx)), y0 = static_cast<int>(std::lround(cy));
        for (int j = -o.rcog; j <= o.rcog; j++)
            for (int i = -o.rcog; i <= o.rcog; i++)
            {
                const int x = x0 + i, y = y0 + j;
                if (i * i + j * j > o.rcog * o.rcog || x < 0 || y < 0 || x >= g.width || y >= g.height)
                    continue;
                const double v = g.at(x, y) - m.background;
                m.peak = std::max(m.peak, g.at(x, y));
                if (v < 3 * m.noise)
                    continue;
                sw += v;
                sx += v * x;
                sy += v * y;
                sxx += v * x * x;
                syy += v * y * y;
            }
        cx = sx / sw;
        cy = sy / sw;
        m.flux = sw;
        mxx = sxx / sw - cx * cx;
        myy = syy / sw - cy * cy;
    }
    m.cogX = cx;
    m.cogY = cy;
    m.sigma = std::sqrt(std::max(0.25, (mxx + myy) / 2));

    // Windowed centroid: x += 2 sum(w I (x - xk)) / sum(w I), Gaussian window of the star's own sigma.
    double ux = cx, uy = cy;
    for (int it = 0; it < 50; it++)
    {
        double sw = 0, dx = 0, dy = 0;
        const int r = static_cast<int>(std::ceil(4 * m.sigma));
        const int x0 = static_cast<int>(std::lround(ux)), y0 = static_cast<int>(std::lround(uy));
        for (int j = -r; j <= r; j++)
            for (int i = -r; i <= r; i++)
            {
                const int x = x0 + i, y = y0 + j;
                if (x < 0 || y < 0 || x >= g.width || y >= g.height)
                    continue;
                const double rx = x - ux, ry = y - uy;
                const double v = (g.at(x, y) - m.background) * std::exp(-(rx * rx + ry * ry) / (2 * m.sigma * m.sigma));
                sw += v;
                dx += v * rx;
                dy += v * ry;
            }
        if (sw <= 0)
            break;
        ux += 2 * dx / sw;
        uy += 2 * dy / sw;
        if (std::hypot(dx, dy) / sw < 1e-5)
            break;
    }
    m.winX = ux;
    m.winY = uy;
    return m;
}

int countSaturated(const common::Frame& raw, double x, double y, int halfSide)
{
    const int top = raw.bitDepth == 8 ? 255 : 65535;
    int n = 0;
    const int xc = static_cast<int>(std::lround(x)), yc = static_cast<int>(std::lround(y));
    for (int j = std::max(0, yc - halfSide); j < std::min(raw.height, yc + halfSide); j++)
        for (int i = std::max(0, xc - halfSide); i < std::min(raw.width, xc + halfSide); i++)
            n += raw.raw(i, j) >= top;
    return n;
}

} // namespace dpot::measure
