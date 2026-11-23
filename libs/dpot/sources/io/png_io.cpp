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

/** @file png_io.cpp
 *  @brief Implementation of png_io.h on stb_image / stb_image_write.
 *  @author Degoras Project Team
 *  @copyright GPL-3.0-or-later
 */

// C++ INCLUDES
#include <algorithm>
#include <vector>

// THIRD PARTY INCLUDES
#include <stb_image.h>
#include <stb_image_write.h>

// PROJECT INCLUDES
#include "io/png_io.h"

// NAMESPACES
namespace dpot::io
{

bool loadFrame(const std::string& path, common::Frame& out, std::string* error)
{
    int w = 0, h = 0, n = 0;
    const bool sixteen = stbi_is_16_bit(path.c_str()) != 0;
    stbi_us* p = stbi_load_16(path.c_str(), &w, &h, &n, 1);   // 8 bit samples come scaled by 257
    if (!p)
    {
        if (error)
            *error = path + ": " + (stbi_failure_reason() ? stbi_failure_reason() : "cannot read");
        return false;
    }
    out.width = w;
    out.height = h;
    out.bitDepth = sixteen ? 16 : 8;
    out.px.assign(p, p + static_cast<std::size_t>(w) * static_cast<std::size_t>(h));
    stbi_image_free(p);
    return true;
}

bool saveFrame8(const std::string& path, const common::Frame& frame)
{
    std::vector<unsigned char> grey(frame.px.size());
    std::transform(frame.px.begin(), frame.px.end(), grey.begin(),
                   [](std::uint16_t v) { return static_cast<unsigned char>((v + 128) / 257); });
    return stbi_write_png(path.c_str(), frame.width, frame.height, 1, grey.data(), frame.width) != 0;
}

} // namespace dpot::io
