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

/** @file canvas.h
 *  @brief An RGB image to draw results on (circles, crosses, text) and save as PNG: the annotated output of the tools.
 *  @author Degoras Project Team
 *  @copyright GPL-3.0-or-later
 */

#pragma once

// C++ INCLUDES
#include <string>
#include <vector>

// PROJECT INCLUDES
#include "common/frame.h"

// NAMESPACES
namespace dpot::io
{

// ---------------------------------------------------------------------------------------------------------------------
// TYPES

struct Rgb
{
    unsigned char r, g, b;
};

namespace colors
{
constexpr Rgb kWhite{255, 255, 255};
constexpr Rgb kRed{255, 40, 40};
constexpr Rgb kGreen{0, 230, 0};
constexpr Rgb kYellow{255, 220, 0};
constexpr Rgb kOrange{255, 140, 0};
constexpr Rgb kCyan{0, 200, 255};
constexpr Rgb kMagenta{255, 0, 255};
} // namespace colors

/// RGB, 8 bits per channel, rows top down (frame pixel convention).
class Canvas
{
public:
    Canvas() = default;
    Canvas(int width, int height);

    int width() const { return width_; }
    int height() const { return height_; }
    std::vector<unsigned char>& rgb() { return rgb_; }
    const std::vector<unsigned char>& rgb() const { return rgb_; }

    void dot(long x, long y, Rgb c);
    void circle(double cx, double cy, double r, int thickness, Rgb c);
    /// Arms of `arm` pixels, leaving `gap` pixels free around the centre.
    void cross(double cx, double cy, int arm, int gap, Rgb c);
    /// Bitmap font (stb_easy_font), each font pixel a scale x scale square, top left at (x, y); '\n' breaks lines.
    void text(double x, double y, const std::string& s, int scale, Rgb c);
    static int textWidth(const std::string& s, int scale);
    /// A square of 2*half pixels around (cx, cy), each pixel drawn zoom x zoom.
    Canvas crop(double cx, double cy, int half, int zoom) const;
    bool savePng(const std::string& path) const;
    /// JPEG, for whole annotated frames (a 6248x4176 RGB PNG is tens of MB).
    bool saveJpg(const std::string& path, int quality = 90) const;

private:
    int width_ = 0;
    int height_ = 0;
    std::vector<unsigned char> rgb_;
};

// ---------------------------------------------------------------------------------------------------------------------
// FUNCTIONS

/// The frame stretched for display (asinh, so faint stars show) from 2 sigma below `background` to 100 sigma above,
/// on the 16 bit scale.
Canvas stretched(const common::Frame& frame, double background, double noise);

/// The same for a float image.
Canvas stretched(const common::FloatImage& image, double background, double noise);

} // namespace dpot::io
