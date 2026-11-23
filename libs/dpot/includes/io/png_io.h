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

/** @file png_io.h
 *  @brief Grey PNG frames as the cameras' capture program (ASICap, RAW8/RAW16) writes them.
 *  @author Degoras Project Team
 *  @copyright GPL-3.0-or-later
 */

#pragma once

// C++ INCLUDES
#include <string>

// PROJECT INCLUDES
#include "common/frame.h"

// NAMESPACES
namespace dpot::io
{

/// Reads a PNG as one grey channel on the 16 bit scale (a colour PNG is turned grey). bitDepth records 8 or 16. False,
/// with the reason in `error`, when the file cannot be read.
bool loadFrame(const std::string& path, common::Frame& out, std::string* error = nullptr);

/// Writes the frame as an 8 bit grey PNG (samples / 257, rounded): lossless for the RAW8 frames the cameras give.
/// ponytail: no 16 bit writer (stb has none); add one if a 16 bit camera's frames must be saved processed.
bool saveFrame8(const std::string& path, const common::Frame& frame);

} // namespace dpot::io
