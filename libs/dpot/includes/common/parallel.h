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

/** @file parallel.h
 *  @brief A minimal parallel for over all hardware threads, for the per-row loops of the detectors.
 *  @author Degoras Project Team
 *  @copyright GPL-3.0-or-later
 */

#pragma once

// C++ INCLUDES
#include <cstddef>
#include <functional>

// NAMESPACES
namespace dpot::common
{

/// Hardware threads (at least 1).
unsigned threadCount();

/// Runs body(i) for every i in [begin, end) on up to threadCount() threads, each taking the next free index, and
/// returns when all are done. body must not throw and must be safe to run concurrently on different indices.
/// ponytail: threads created per call (~0.1 ms); a persistent pool if a frame pipeline needs those back.
void parallelFor(std::size_t begin, std::size_t end, const std::function<void(std::size_t)>& body);

} // namespace dpot::common
