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

/** @file star_measure.cpp
 *  @brief Implementation of star_measure.h: hfd() from faster-astap on a window cut from the frame.
 *  @author Degoras Project Team
 *  @copyright GPL-3.0-or-later
 */

// C++ INCLUDES
#include <initializer_list>
#include <algorithm>
#include <cmath>
#include <vector>

// THIRD PARTY INCLUDES
#include "astap/star_detection.h"
#include "astap/types.h"

// PROJECT INCLUDES
#include "common/stats.h"
#include "measure/star_measure.h"

// NAMESPACES
namespace dpot::measure
{

namespace
{

constexpr int kHeavySaturation = 100;   // above this many saturated pixels a star is measured as a blob
constexpr int kSearch = 8;              // px around the prediction where the peak is looked for
constexpr int kWin = 50;                // half size of the background window
constexpr int kR = 80;                  // half size of the window cut from the frame: kWin + kSearch + hfd()'s box

// On `img` (faster-astap's convention: rows bottom up, 0-based), around (x, y) in the same convention.
bool measureIn(const astap::ImageArray& img, double x, double y, StarMeasurement& m)
{
    const int cx = static_cast<int>(std::lround(x)), cy = static_cast<int>(std::lround(y));
    if (cx - kWin - kSearch < 1 || cy - kWin - kSearch < 1 || cx + kWin + kSearch >= img.width() - 1 ||
        cy + kWin + kSearch >= img.height() - 1)
        return false;
    std::vector<float> border;
    for (int i = -kWin; i <= kWin; i++)
        for (float v : {img.at(0, cy - kWin, cx + i), img.at(0, cy + kWin, cx + i), img.at(0, cy + i, cx - kWin),
                        img.at(0, cy + i, cx + kWin)})
            border.push_back(v);
    const float bg = common::median(border);
    const double sd = std::max(static_cast<double>(common::madSigma(border, bg)), 1.0);

    double peak = -1;
    int px = cx, py = cy;
    for (int j = -kSearch; j <= kSearch; j++)
        for (int i = -kSearch; i <= kSearch; i++)
        {
            double s = 0;
            for (int b = -1; b <= 1; b++)
                for (int a = -1; a <= 1; a++)
                    s += img.at(0, cy + j + b, cx + i + a);
            if (s > peak)
            {
                peak = s;
                px = cx + i;
                py = cy + j;
            }
        }
    if (peak / 9 < bg + 10 * sd)
        return false;

    m.saturated = 0;
    for (int j = -kWin; j <= kWin; j++)
        for (int i = -kWin; i <= kWin; i++)
            if (img.at(0, py + j, px + i) >= 65535)
                m.saturated++;
    m.blob = m.saturated > kHeavySaturation;
    m.snr = 0;
    if (m.blob)
    {
        double sx = px, sy = py;
        for (int pass = 0; pass < 2; pass++)   // the second recentres the window
        {
            const int x0 = static_cast<int>(std::lround(sx)), y0 = static_cast<int>(std::lround(sy));
            const int xa = std::max(0, x0 - kWin), xb = std::min(img.width() - 1, x0 + kWin);
            const int ya = std::max(0, y0 - kWin), yb = std::min(img.height() - 1, y0 + kWin);
            float top = 0;
            for (int j = ya; j <= yb; j++)
                for (int i = xa; i <= xb; i++)
                    top = std::max(top, img.at(0, j, i));
            const double level = bg + 0.5 * (top - bg);
            double sw = 0, wx = 0, wy = 0;
            for (int j = ya; j <= yb; j++)
                for (int i = xa; i <= xb; i++)
                {
                    const double v = img.at(0, j, i);
                    if (v < level)
                        continue;
                    sw += v - bg;
                    wx += (v - bg) * i;
                    wy += (v - bg) * j;
                }
            sx = wx / sw;
            sy = wy / sw;
        }
        m.x = sx;
        m.y = sy;
        return true;
    }
    double hfd1, fwhm, flux;
    astap::hfd(img, px, py, 14, hfd1, fwhm, m.snr, flux, m.x, m.y);
    if (hfd1 >= 999)
        return false;
    astap::hfd(img, static_cast<int>(std::lround(m.x)), static_cast<int>(std::lround(m.y)), 14, hfd1, fwhm, m.snr,
               flux, m.x, m.y);
    return hfd1 < 999;
}

} // namespace

bool measureStar(const common::Frame& frame, double x, double y, StarMeasurement& out)
{
    // The window, in faster-astap's convention (rows bottom up): yb is the prediction's row counted from the bottom.
    const double yb = frame.height - 1 - y;
    const int cx = static_cast<int>(std::lround(x)), cy = static_cast<int>(std::lround(yb));
    if (cx - kR < 0 || cy - kR < 0 || cx + kR >= frame.width || cy + kR >= frame.height)
        return false;
    astap::ImageArray win(1, 2 * kR + 1, 2 * kR + 1);
    for (int j = 0; j <= 2 * kR; j++)
    {
        const std::uint16_t* src = frame.row(frame.height - 1 - (cy - kR + j)) + cx - kR;
        std::copy(src, src + 2 * kR + 1, win.row(0, j));
    }
    StarMeasurement m;
    if (!measureIn(win, x - (cx - kR), yb - (cy - kR), m))
        return false;
    m.x += cx - kR;
    m.y = frame.height - 1 - (m.y + cy - kR);   // back to frame pixels, rows top down
    out = m;
    return true;
}

} // namespace dpot::measure
