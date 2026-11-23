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

/** @file background.h
 *  @brief Sky background and noise on a grid of cells, and running box means.
 *  @author Degoras Project Team
 *  @copyright GPL-3.0-or-later
 */

#pragma once

// C++ INCLUDES
#include <cstddef>
#include <vector>

// PROJECT INCLUDES
#include "common/frame.h"

// NAMESPACES
namespace dpot::detection
{

/// Background (median) and noise (MAD sigma, at least 1) per kCell x kCell square, from every kStride-th pixel; cells
/// in parallel. Follows gradients such as moonlight at the scale of a cell (128 px on a 3x3 binned RASA frame = 12').
struct BackgroundGrid
{
    static constexpr int kCell = 128;
    static constexpr int kStride = 4;

    int nx = 0;
    int ny = 0;
    std::vector<float> level;
    std::vector<float> noise;

    explicit BackgroundGrid(const common::FloatImage& image);

    std::size_t cellOf(int x, int y) const
    {
        return static_cast<std::size_t>(y / kCell) * static_cast<std::size_t>(nx) + static_cast<std::size_t>(x / kCell);
    }
};

/// Mean over a (2r+1)^2 box through an integral image; at the edges, of the part of the box inside the image.
common::FloatImage boxMean(const common::FloatImage& image, int r);

} // namespace dpot::detection
