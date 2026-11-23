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

/** @file binning.cpp
 *  @brief Implementation of binning.h.
 *  @author Degoras Project Team
 *  @copyright GPL-3.0-or-later
 */

// C++ INCLUDES
#include <cstdint>
#include <vector>

// PROJECT INCLUDES
#include "common/parallel.h"
#include "detection/binning.h"

// NAMESPACES
namespace dpot::detection
{

common::FloatImage binFrame(const common::Frame& frame, int bin, double crop, BinnedGeometry& geometry)
{
    const int cw = static_cast<int>(frame.width * crop) / bin * bin;
    const int ch = static_cast<int>(frame.height * crop) / bin * bin;
    geometry.bin = bin;
    geometry.x0 = (frame.width - cw) / 2;
    geometry.y0 = (frame.height - ch) / 2;
    common::FloatImage out(cw / bin, ch / bin);
    const int ow = out.width;
    common::parallelFor(0, static_cast<std::size_t>(out.height), [&](std::size_t y) {
        std::vector<std::uint32_t> acc(static_cast<std::size_t>(ow), 0);
        for (int j = 0; j < bin; j++)
        {
            const std::uint16_t* src = frame.row(geometry.y0 + static_cast<int>(y) * bin + j) + geometry.x0;
            for (int x = 0; x < ow; x++)
                for (int i = 0; i < bin; i++)
                    acc[static_cast<std::size_t>(x)] += src[x * bin + i];
        }
        float* dst = out.row(static_cast<int>(y));
        for (int x = 0; x < ow; x++)
            dst[x] = static_cast<float>(acc[static_cast<std::size_t>(x)]) / static_cast<float>(bin * bin);
    });
    return out;
}

} // namespace dpot::detection
