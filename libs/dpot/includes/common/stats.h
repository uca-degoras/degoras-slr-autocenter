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

/** @file stats.h
 *  @brief The robust statistics the detectors use: median, MAD as a standard deviation, mean, deviation and drift.
 *  @author Degoras Project Team
 *  @copyright GPL-3.0-or-later
 */

#pragma once

// C++ INCLUDES
#include <vector>

// NAMESPACES
namespace dpot::common
{

// ---------------------------------------------------------------------------------------------------------------------
// FUNCTIONS

/// Median (the upper one for an even count); 0 for no values. Takes a copy: nth_element reorders it.
float median(std::vector<float> values);

/// 1.4826 times the median absolute deviation from `centre`: the standard deviation of Gaussian noise, unmoved by
/// stars. 0 for no values.
float madSigma(std::vector<float> values, float centre);

double mean(const std::vector<double>& values);

/// Sample standard deviation (n - 1); 0 for fewer than two values.
double stdev(const std::vector<double>& values);

/// Least squares slope of the values against their index: the drift per frame of a measured position.
double slope(const std::vector<double>& values);

} // namespace dpot::common
