/***********************************************************************************************************************
 *  DegorasOpticalTracking_PoF - Degoras Optical Tracking: real-time detection of stars, satellites and the laser
 *  beam in the cameras of the SFEL station.
 *
 *  Copyright (C) 2026 Degoras Project Team
 *
 *  SPDX-License-Identifier: GPL-3.0-or-later
 **********************************************************************************************************************/

/** @file check.h
 *  @brief The tests' assertions: CHECK(condition) and CHECK_NEAR(a, b, tolerance) print what failed and where, and
 *         exit with 1; active in Release too. A test that has no data to run on exits with 77 (CTest: skipped).
 */

#pragma once

// C++ INCLUDES
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <filesystem>
#include <string>

#define CHECK(cond)                                                                                                  \
    do                                                                                                               \
    {                                                                                                                \
        if (!(cond))                                                                                                 \
        {                                                                                                            \
            std::fprintf(stderr, "%s:%d: CHECK(%s) failed\n", __FILE__, __LINE__, #cond);                            \
            std::exit(1);                                                                                            \
        }                                                                                                            \
    } while (false)

#define CHECK_NEAR(a, b, tol)                                                                                        \
    do                                                                                                               \
    {                                                                                                                \
        const double _a = (a), _b = (b);                                                                             \
        if (!(std::fabs(_a - _b) <= (tol)))                                                                          \
        {                                                                                                            \
            std::fprintf(stderr, "%s:%d: CHECK_NEAR(%s, %s, %s) failed: %.10g vs %.10g\n", __FILE__, __LINE__, #a,  \
                         #b, #tol, _a, _b);                                                                          \
            std::exit(1);                                                                                            \
        }                                                                                                            \
    } while (false)

namespace dpot::testing
{

/// Exits 77 (skipped) when `path` does not exist, saying what is missing.
inline void requireData(const std::string& path, const char* what)
{
    if (!std::filesystem::exists(path))
    {
        std::printf("SKIPPED: no %s at %s\n", what, path.c_str());
        std::exit(77);
    }
}

} // namespace dpot::testing
