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

/** @file capture_time.cpp
 *  @brief Implementation of capture_time.h.
 *  @author Degoras Project Team
 *  @copyright GPL-3.0-or-later
 */

// C++ INCLUDES
#include <cstdio>
#include <filesystem>
#include <fstream>
#include <map>
#include <mutex>

// PROJECT INCLUDES
#include "io/capture_time.h"

// NAMESPACES
namespace dpot::io
{

namespace
{

namespace fs = std::filesystem;

// The frame_times.csv files read so far, by path: a run asks for every frame of the same folder.
// ponytail: kept for the life of the program; a few thousand lines each.
std::map<std::string, std::map<std::string, common::UtcTime>>& csvCache()
{
    static std::map<std::string, std::map<std::string, common::UtcTime>> cache;
    return cache;
}

const std::map<std::string, common::UtcTime>& readCsv(const fs::path& csv)
{
    static std::mutex mutex;
    const std::lock_guard<std::mutex> lock(mutex);
    auto& cache = csvCache();
    const std::string key = csv.generic_string();
    if (const auto it = cache.find(key); it != cache.end())
        return it->second;
    std::map<std::string, common::UtcTime>& rows = cache[key];
    std::ifstream in(csv);
    for (std::string line; std::getline(in, line);)
    {
        const std::size_t comma = line.find(',');
        common::UtcTime t;
        if (comma != std::string::npos && common::parseIso(line.substr(comma + 1), t))
            rows[line.substr(0, comma)] = t;
    }
    return rows;
}

} // namespace

const char* toString(TimeSource source)
{
    switch (source)
    {
        case TimeSource::FILE_NAME: return "file name (to the minute)";
        case TimeSource::SESSION: return "session folder (capture start, to the second)";
        case TimeSource::FRAME_TIMES: return "frame_times.csv (to the millisecond)";
        default: return "none";
    }
}

TimeSource captureTime(const std::string& path, common::UtcTime& out)
{
    const fs::path frame = fs::absolute(fs::path(path)).lexically_normal();

    // 1. frame_times.csv in a folder above the frame, keyed by the path relative to that folder.
    for (fs::path dir = frame.parent_path(); !dir.empty(); dir = dir.parent_path())
    {
        const fs::path csv = dir / "frame_times.csv";
        if (fs::exists(csv))
        {
            const auto& rows = readCsv(csv);
            if (const auto it = rows.find(frame.lexically_relative(dir).generic_string()); it != rows.end())
            {
                out = it->second;
                return TimeSource::FRAME_TIMES;
            }
            break;
        }
        if (dir == dir.root_path())
            break;
    }

    common::UtcTime t;
    // 2. The session folder, "2026-10-28_19_24_41Z".
    const std::string folder = frame.parent_path().filename().string();
    int sec = 0;
    if (std::sscanf(folder.c_str(), "%d-%d-%d_%d_%d_%dZ", &t.year, &t.month, &t.day, &t.hour, &t.minute, &sec) == 6)
    {
        t.second = sec;
        out = t;
        return TimeSource::SESSION;
    }
    // 3. The file name, "2026-10-28-1924_4-CapObj_0033.PNG".
    const std::string name = frame.filename().string();
    int hhmm = 0;
    if (std::sscanf(name.c_str(), "%d-%d-%d-%4d", &t.year, &t.month, &t.day, &hhmm) == 4)
    {
        t.hour = hhmm / 100;
        t.minute = hhmm % 100;
        t.second = 0;
        out = t;
        return TimeSource::FILE_NAME;
    }
    return TimeSource::NONE;
}

} // namespace dpot::io
