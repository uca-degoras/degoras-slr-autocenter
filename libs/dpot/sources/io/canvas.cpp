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

/** @file canvas.cpp
 *  @brief Implementation of canvas.h.
 *  @author Degoras Project Team
 *  @copyright GPL-3.0-or-later
 */

// C++ INCLUDES
#include <algorithm>
#include <cmath>

// THIRD PARTY INCLUDES
#include <stb_easy_font.h>
#include <stb_image_write.h>

// PROJECT INCLUDES
#include "common/angles.h"
#include "common/parallel.h"
#include "io/canvas.h"

// NAMESPACES
namespace dpot::io
{

namespace
{

unsigned char stretch(double v, double lo, double span)
{
    return static_cast<unsigned char>(255 * std::asinh(20 * std::clamp((v - lo) / span, 0.0, 1.0)) / std::asinh(20.0));
}

} // namespace

Canvas::Canvas(int width, int height) :
    width_(width), height_(height), rgb_(static_cast<std::size_t>(width) * static_cast<std::size_t>(height) * 3)
{}

void Canvas::dot(long x, long y, Rgb c)
{
    if (x < 0 || y < 0 || x >= width_ || y >= height_)
        return;
    const auto k = static_cast<std::size_t>(y) * static_cast<std::size_t>(width_) + static_cast<std::size_t>(x);
    unsigned char* p = &rgb_[k * 3];
    p[0] = c.r;
    p[1] = c.g;
    p[2] = c.b;
}

void Canvas::circle(double cx, double cy, double r, int thickness, Rgb c)
{
    for (int k = 0; k < thickness; k++)
    {
        const int n = static_cast<int>(4 * common::kPi * (r + k)) + 8;
        for (int i = 0; i < n; i++)
            dot(std::lround(cx + (r + k) * std::cos(2 * common::kPi * i / n)),
                std::lround(cy + (r + k) * std::sin(2 * common::kPi * i / n)), c);
    }
}

void Canvas::cross(double cx, double cy, int arm, int gap, Rgb c)
{
    const long x = std::lround(cx), y = std::lround(cy);
    if (gap == 0)
        dot(x, y, c);
    for (int i = std::max(gap, 1); i <= arm; i++)
    {
        dot(x + i, y, c);
        dot(x - i, y, c);
        dot(x, y + i, c);
        dot(x, y - i, c);
    }
}

void Canvas::text(double x, double y, const std::string& s, int scale, Rgb c)
{
    // A quad is 4 vertices of x, y, z and 4 colour bytes; up to ~16 quads per character.
    std::vector<float> v(256 * (s.size() + 1));
    std::string copy = s;
    const int quads = stb_easy_font_print(0, 0, copy.data(), nullptr, v.data(), static_cast<int>(v.size() * 4));
    for (int q = 0; q < quads; q++)
    {
        const float* a = &v[static_cast<std::size_t>(16 * q)];   // vertices 0 and 2: opposite corners
        const float* b = &v[static_cast<std::size_t>(16 * q + 8)];
        for (long j = std::lround(y + a[1] * scale); j < std::lround(y + b[1] * scale); j++)
            for (long i = std::lround(x + a[0] * scale); i < std::lround(x + b[0] * scale); i++)
                dot(i, j, c);
    }
}

int Canvas::textWidth(const std::string& s, int scale)
{
    std::string copy = s;
    return stb_easy_font_width(copy.data()) * scale;
}

Canvas Canvas::crop(double cx, double cy, int half, int zoom) const
{
    const int n = 2 * half * zoom;
    Canvas out(n, n);
    for (int y = 0; y < n; y++)
        for (int x = 0; x < n; x++)
        {
            const long sx = std::lround(cx) - half + x / zoom, sy = std::lround(cy) - half + y / zoom;
            if (sx < 0 || sy < 0 || sx >= width_ || sy >= height_)
                continue;
            const auto from =
                static_cast<std::size_t>(sy) * static_cast<std::size_t>(width_) + static_cast<std::size_t>(sx);
            const auto to = static_cast<std::size_t>(y) * static_cast<std::size_t>(n) + static_cast<std::size_t>(x);
            std::copy_n(&rgb_[from * 3], 3, &out.rgb_[to * 3]);
        }
    return out;
}

bool Canvas::savePng(const std::string& path) const
{
    return stbi_write_png(path.c_str(), width_, height_, 3, rgb_.data(), width_ * 3) != 0;
}

bool Canvas::saveJpg(const std::string& path, int quality) const
{
    return stbi_write_jpg(path.c_str(), width_, height_, 3, rgb_.data(), quality) != 0;
}

Canvas stretched(const common::Frame& frame, double background, double noise)
{
    const double lo = background - 2 * noise, span = 100 * noise;
    std::vector<unsigned char> lut(65536);
    for (int v = 0; v < 65536; v++)
        lut[static_cast<std::size_t>(v)] = stretch(v, lo, span);
    Canvas c(frame.width, frame.height);
    common::parallelFor(0, static_cast<std::size_t>(frame.height), [&](std::size_t y) {
        unsigned char* dst = &c.rgb()[y * static_cast<std::size_t>(frame.width) * 3];
        const std::uint16_t* src = frame.row(static_cast<int>(y));
        for (int x = 0; x < frame.width; x++)
            std::fill_n(dst + 3 * x, 3, lut[src[x]]);
    });
    return c;
}

Canvas stretched(const common::FloatImage& image, double background, double noise)
{
    const double lo = background - 2 * noise, span = 100 * noise;
    Canvas c(image.width, image.height);
    common::parallelFor(0, static_cast<std::size_t>(image.height), [&](std::size_t y) {
        unsigned char* dst = &c.rgb()[y * static_cast<std::size_t>(image.width) * 3];
        const float* src = image.row(static_cast<int>(y));
        for (int x = 0; x < image.width; x++)
            std::fill_n(dst + 3 * x, 3, stretch(src[x], lo, span));
    });
    return c;
}

} // namespace dpot::io
