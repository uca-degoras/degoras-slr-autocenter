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

/** @file parallel.cpp
 *  @brief Implementation of parallel.h.
 *  @author Degoras Project Team
 *  @copyright GPL-3.0-or-later
 */

// C++ INCLUDES
#include <algorithm>
#include <atomic>
#include <thread>
#include <vector>

// PROJECT INCLUDES
#include "common/parallel.h"

// NAMESPACES
namespace dpot::common
{

unsigned threadCount()
{
    return std::max(1u, std::thread::hardware_concurrency());
}

void parallelFor(std::size_t begin, std::size_t end, const std::function<void(std::size_t)>& body)
{
    if (end <= begin)
        return;
    const std::size_t workers = std::min<std::size_t>(threadCount(), end - begin);
    std::atomic<std::size_t> next{begin};
    auto work = [&] {
        for (std::size_t i = next++; i < end; i = next++)
            body(i);
    };
    std::vector<std::thread> threads;
    threads.reserve(workers - 1);
    for (std::size_t t = 1; t < workers; t++)
        threads.emplace_back(work);
    work();
    for (std::thread& t : threads)
        t.join();
}

} // namespace dpot::common
