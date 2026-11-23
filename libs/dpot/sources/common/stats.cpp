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

/** @file stats.cpp
 *  @brief Implementation of stats.h.
 *  @author Degoras Project Team
 *  @copyright GPL-3.0-or-later
 */

// C++ INCLUDES
#include <algorithm>
#include <cmath>
#include <utility>

// PROJECT INCLUDES
#include "common/stats.h"

// NAMESPACES
namespace dpot::common
{

float median(std::vector<float> values)
{
    if (values.empty())
        return 0;
    const auto mid = values.begin() + static_cast<long>(values.size() / 2);
    std::nth_element(values.begin(), mid, values.end());
    return *mid;
}

float madSigma(std::vector<float> values, float centre)
{
    for (float& v : values)
        v = std::fabs(v - centre);
    return 1.4826f * median(std::move(values));
}

double mean(const std::vector<double>& values)
{
    double s = 0;
    for (double v : values)
        s += v;
    return values.empty() ? 0 : s / static_cast<double>(values.size());
}

double stdev(const std::vector<double>& values)
{
    if (values.size() < 2)
        return 0;
    const double m = mean(values);
    double s = 0;
    for (double v : values)
        s += (v - m) * (v - m);
    return std::sqrt(s / static_cast<double>(values.size() - 1));
}

double slope(const std::vector<double>& values)
{
    const double mi = (static_cast<double>(values.size()) - 1) / 2, mv = mean(values);
    double sxy = 0, sxx = 0;
    for (std::size_t i = 0; i < values.size(); i++)
    {
        sxy += (static_cast<double>(i) - mi) * (values[i] - mv);
        sxx += (static_cast<double>(i) - mi) * (static_cast<double>(i) - mi);
    }
    return sxx > 0 ? sxy / sxx : 0;
}

} // namespace dpot::common
