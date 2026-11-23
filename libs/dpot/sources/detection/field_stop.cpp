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

/** @file field_stop.cpp
 *  @brief Implementation of field_stop.h.
 *  @author Degoras Project Team
 *  @copyright GPL-3.0-or-later
 */

// C++ INCLUDES
#include <initializer_list>
#include <cmath>
#include <utility>

// PROJECT INCLUDES
#include "common/stats.h"
#include "detection/background.h"
#include "detection/field_stop.h"

// NAMESPACES
namespace dpot::detection
{

FieldMask fieldMask(const common::FloatImage& plane)
{
    const int w = plane.width, h = plane.height;
    std::vector<float> dark, mid;
    for (int y = 0; y < 40; y++)
        for (int x = 0; x < 40; x++)
            for (const auto& [cx, cy] : {std::pair{x, y}, {w - 1 - x, y}, {x, h - 1 - y}, {w - 1 - x, h - 1 - y}})
                dark.push_back(plane.at(cx, cy));
    for (int y = h / 2 - 100; y < h / 2 + 100; y++)
        for (int x = w / 2 - 100; x < w / 2 + 100; x++)
            mid.push_back(plane.at(x, y));
    const float level = (common::median(dark) + common::median(mid)) / 2;
    FieldMask m;
    m.field.resize(plane.px.size());
    common::FloatImage f(w, h);
    for (std::size_t i = 0; i < plane.px.size(); i++)
    {
        m.field[i] = plane.px[i] > level;
        f.px[i] = m.field[i];
    }
    const common::FloatImage near = boxMean(f, 45);
    m.inner.resize(plane.px.size());
    for (std::size_t i = 0; i < plane.px.size(); i++)
        m.inner[i] = near.px[i] > 0.999f;
    return m;
}

bool fitFieldStop(const FieldMask& mask, int width, int height, double& cx, double& cy, double& radius)
{
    const std::vector<char>& field = mask.field;
    const std::size_t w = static_cast<std::size_t>(width);
    double m[3][4] = {};
    for (int y = 2; y < height - 2; y++)
        for (int x = 2; x < width - 2; x++)
        {
            const std::size_t k = static_cast<std::size_t>(y) * w + static_cast<std::size_t>(x);
            if (!field[k] || (field[k - 1] && field[k + 1] && field[k - w] && field[k + w]))
                continue;   // not an edge pixel
            const double v[3] = {static_cast<double>(x), static_cast<double>(y), 1};
            const double z = -(static_cast<double>(x) * x + static_cast<double>(y) * y);
            for (int i = 0; i < 3; i++)
            {
                for (int j = 0; j < 3; j++)
                    m[i][j] += v[i] * v[j];
                m[i][3] += v[i] * z;
            }
        }
    for (int i = 0; i < 3; i++)   // Gauss-Jordan on the 3x3 normal equations
    {
        if (std::fabs(m[i][i]) < 1e-9)
            return false;
        for (int j = 0; j < 3; j++)
            if (j != i)
            {
                const double f = m[j][i] / m[i][i];
                for (int c = 0; c < 4; c++)
                    m[j][c] -= f * m[i][c];
            }
    }
    const double d = m[0][3] / m[0][0], e = m[1][3] / m[1][1], f = m[2][3] / m[2][2];
    cx = -d / 2;
    cy = -e / 2;
    radius = std::sqrt(cx * cx + cy * cy - f);
    return std::isfinite(radius);
}

} // namespace dpot::detection
