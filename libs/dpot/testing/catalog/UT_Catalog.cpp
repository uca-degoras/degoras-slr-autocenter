/***********************************************************************************************************************
 *  DegorasOpticalTracking_PoF - Degoras Optical Tracking: real-time detection of stars, satellites and the laser
 *  beam in the cameras of the SFEL station.
 *
 *  Copyright (C) 2026 Degoras Project Team
 *
 *  SPDX-License-Identifier: GPL-3.0-or-later
 **********************************************************************************************************************/

/** @file UT_Catalog.cpp
 *  @brief Unit test of the catalog layer on the files in data/catalogs: FK5 (both parts) and the IAU names.
 */

// PROJECT INCLUDES
#include "catalog/fk5.h"
#include "catalog/star_names.h"
#include "support/check.h"

using namespace dpot;
using common::kArcsecToRad;
using common::kDegToRad;

int main()
{
    const std::string dir = std::string(DPOT_SOURCE_DIR) + "/data/catalogs";
    std::vector<catalog::Fk5Star> fk5;
    std::string error;
    CHECK(catalog::readFk5(dir, fk5, &error));
    CHECK(fk5.size() == 1535 + 3117);
    CHECK(!catalog::readFk5(dir + "/missing", fk5, &error) && !error.empty());

    // Matar (eta Peg), I/149A: 22 43 00.143 +30 13 16.52, pm +0.113 s/cy, -2.54 "/cy, HD 215182.
    const catalog::Fk5Star* matar = catalog::findFk5(fk5, 857);
    CHECK(matar != nullptr);
    CHECK_NEAR(matar->ra, (22 + 43 / 60.0 + 0.143 / 3600) * 15 * kDegToRad, 1e-9);
    CHECK_NEAR(matar->dec, (30 + 13 / 60.0 + 16.52 / 3600) * kDegToRad, 1e-9);
    CHECK_NEAR(matar->pmRa, 0.113 * 15 * kArcsecToRad / 100, 1e-12);
    CHECK_NEAR(matar->pmDec, -2.54 * kArcsecToRad / 100, 1e-12);
    CHECK(matar->hd == 215182);
    CHECK(catalog::findFk5(fk5, 99999) == nullptr);

    // Proper motion: Altair moves 0.39"/yr in Dec.
    const catalog::Fk5Star* altair = catalog::findFk5(fk5, 745);
    CHECK(altair != nullptr);
    const common::RaDec p = catalog::positionAt(*altair, 26.82);
    CHECK_NEAR((p.dec - altair->dec) / kArcsecToRad, 0.3863 * 26.82, 0.05);

    // Proper names through HD; FK5 1481 (the experiment's faint target) has none.
    const auto names = catalog::readIauNames(dir + "/names/IAU-CSN.txt");
    CHECK(names.size() > 400);
    CHECK(catalog::fk5Label(*matar, names) == "FK5 857\nMatar");
    CHECK(catalog::fk5Label(*altair, names) == "FK5 745\nAltair");
    CHECK(catalog::fk5Label(*catalog::findFk5(fk5, 907), names) == "FK5 907\nPolaris");
    CHECK(catalog::fk5Label(*catalog::findFk5(fk5, 1481), names) == "FK5 1481");
    return 0;
}
