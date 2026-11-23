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

/** @file frame.h
 *  @brief The images the project works on: a camera Frame (16 bit samples) and a FloatImage (processed data).
 *
 *  PIXEL CONVENTION, the same everywhere in this project: frame pixels, 0-based, x to the right, y DOWN, the centre of
 *  the top-left pixel at (0, 0), as the camera and the PNG files deliver them. Only sources/solve and sources/measure
 *  turn them into the FITS convention faster-astap uses (rows bottom up, 1-based), and back.
 *  @author Degoras Project Team
 *  @copyright GPL-3.0-or-later
 */

#pragma once

// C++ INCLUDES
#include <cstddef>
#include <cstdint>
#include <vector>

// NAMESPACES
namespace dpot::common
{

// ---------------------------------------------------------------------------------------------------------------------
// TYPES

/// One camera frame, one channel, rows top down. Samples are on a 16 bit scale: 8 bit data comes multiplied by 257 (as
/// faster-astap and stb read it, so 255 becomes 65535), and bitDepth keeps the depth the camera gave.
struct Frame
{
    int width = 0;
    int height = 0;
    int bitDepth = 16;
    std::vector<std::uint16_t> px;

    const std::uint16_t* row(int y) const { return px.data() + static_cast<std::size_t>(y) * width; }
    std::uint16_t* row(int y) { return px.data() + static_cast<std::size_t>(y) * width; }
    std::uint16_t at(int x, int y) const { return row(y)[x]; }
    /// The sample as the camera gave it: 0..255 for 8 bit data, 0..65535 for 16 bit.
    int raw(int x, int y) const { return bitDepth == 8 ? at(x, y) / 257 : at(x, y); }
};

/// A float image, rows top down: binned frames, Bayer sums, colour planes, maps.
struct FloatImage
{
    int width = 0;
    int height = 0;
    std::vector<float> px;

    FloatImage() = default;
    FloatImage(int w, int h, float value = 0) :
        width(w), height(h), px(static_cast<std::size_t>(w) * static_cast<std::size_t>(h), value) {}

    const float* row(int y) const { return px.data() + static_cast<std::size_t>(y) * width; }
    float* row(int y) { return px.data() + static_cast<std::size_t>(y) * width; }
    float at(int x, int y) const { return row(y)[x]; }
    float& at(int x, int y) { return row(y)[x]; }
};

} // namespace dpot::common
