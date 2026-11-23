/***********************************************************************************************************************
 *  DegorasOpticalTracking_PoF - Degoras Optical Tracking: real-time detection of stars, satellites and the laser
 *  beam in the cameras of the SFEL station.
 *
 *  Copyright (C) 2026 Degoras Project Team
 *
 *  SPDX-License-Identifier: GPL-3.0-or-later
 **********************************************************************************************************************/

/** @file UT_SolverOpen.cpp
 *  @brief Unit test of the solve layer without a star database: a missing database is reported as such, and the status
 *         names are stable.
 */

// C++ INCLUDES
#include <string>

// PROJECT INCLUDES
#include "solve/solver.h"
#include "support/check.h"

using namespace dpot;

int main()
{
    solve::SolverConfig config;
    config.databaseDir = std::string(DPOT_TEST_TMP_DIR) + "/no_database_here";
    std::unique_ptr<solve::Solver> solver;
    std::string message;
    CHECK(solve::Solver::open(config, solver, &message) == solve::Status::CATALOG_NOT_FOUND);
    CHECK(!solver && message.find("d80") != std::string::npos);
    CHECK(std::string(solve::toString(solve::Status::NO_SOLUTION)) == "NO_SOLUTION");
    CHECK(static_cast<int>(solve::Status::TOO_FEW_STARS) == 2);   // ASTAP's exit code
    CHECK(static_cast<int>(solve::Status::CATALOG_NOT_FOUND) == 32);
    return 0;
}
