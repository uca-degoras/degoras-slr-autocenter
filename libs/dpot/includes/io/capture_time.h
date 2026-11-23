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

/** @file capture_time.h
 *  @brief The UTC a frame was taken, for proper motions and pointing.
 *
 *  Three sources, best first:
 *   1. frame_times.csv in the images folder (or any folder above the frame): "<path relative to that folder>,<ISO UTC>"
 *      per frame, with the time each PNG was written, to the millisecond. The package ships one, made from the PNG
 *      file times, because unzipping does not keep them.
 *   2. The ASICap session folder the frame sits in, "YYYY-MM-DD_HH_MM_SSZ": when the capture started, to the second.
 *   3. The ASICap file name, "YYYY-MM-DD-HHMM_...": to the minute.
 *  @author Degoras Project Team
 *  @copyright GPL-3.0-or-later
 */

#pragma once

// C++ INCLUDES
#include <string>

// PROJECT INCLUDES
#include "common/utc_time.h"

// NAMESPACES
namespace dpot::io
{

/// How precise the time captureTime() found is.
enum class TimeSource
{
    NONE,          ///< Nothing usable
    FILE_NAME,     ///< To the minute
    SESSION,       ///< Start of the capture session, to the second
    FRAME_TIMES    ///< The frame's own write time, to the millisecond
};

const char* toString(TimeSource source);

/// The best time available for the frame at `path`. Returns where it came from (NONE leaves `out` untouched).
TimeSource captureTime(const std::string& path, common::UtcTime& out);

} // namespace dpot::io
