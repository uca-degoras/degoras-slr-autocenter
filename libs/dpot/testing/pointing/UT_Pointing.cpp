/***********************************************************************************************************************
 *  DegorasOpticalTracking_PoF - Degoras Optical Tracking: real-time detection of stars, satellites and the laser
 *  beam in the cameras of the SFEL station.
 *
 *  Copyright (C) 2026 Degoras Project Team
 *
 *  SPDX-License-Identifier: GPL-3.0-or-later
 **********************************************************************************************************************/

/** @file UT_Pointing.cpp
 *  @brief Unit test of the pointing layer against Meeus, Astronomical Algorithms (2nd ed.): sidereal time (12.a, 12.b),
 *         precession with proper motion (21.b), horizontal coordinates (13.b); and the camera model and TPOINT pair.
 */

// C++ INCLUDES
#include <initializer_list>
#include <cmath>

// PROJECT INCLUDES
#include "pointing/camera_model.h"
#include "pointing/horizon.h"
#include "support/check.h"

using namespace dpot;
using common::kArcsecToRad;
using common::kDegToRad;
using common::kRadToDeg;

int main()
{
    // 12.a: 1987 April 10, 0h UT -> 13h 10m 46.3668s. 12.b: 19h 21m 00s UT -> 8h 34m 57.0896s.
    CHECK_NEAR(pointing::greenwichMeanSiderealTime(2446895.5) * kRadToDeg, (13 + 10 / 60.0 + 46.3668 / 3600) * 15,
               1e-5);
    CHECK_NEAR(pointing::greenwichMeanSiderealTime(common::julianDate({1987, 4, 10, 19, 21, 0})) * kRadToDeg,
               (8 + 34 / 60.0 + 57.0896 / 3600) * 15, 1e-5);

    // 21.b: theta Persei from J2000 to 2028 November 13.19 (JD 2462088.69), proper motion included:
    // 2h 44m 11.986s +49 13' 42.48", pm +0.03425 s/yr, -0.0895 "/yr -> 41.547214 deg, +49.348483 deg.
    const double t = (2462088.69 - 2451545.0) / 36525;
    const common::RaDec j2000{(2 + 44 / 60.0 + (11.986 + 0.03425 * t * 100) / 3600) * 15 * kDegToRad,
                              (49 + 13 / 60.0 + (42.48 - 0.0895 * t * 100) / 3600) * kDegToRad};
    const common::RaDec date = pointing::precessFromJ2000(j2000, t);
    CHECK_NEAR(date.ra * kRadToDeg, 41.547214, 1e-5);
    CHECK_NEAR(date.dec * kRadToDeg, 49.348483, 1e-5);

    // 13.b: H = 64.352133 deg, dec -6 43' 11.61", latitude +38 55' 17" -> A = 68.0337 deg from the SOUTH (248.0337
    // from the north), h = 15.1249 deg.
    double az = 0, el = 0;
    const double lat = (38 + 55 / 60.0 + 17 / 3600.0) * kDegToRad, dec = -(6 + 43 / 60.0 + 11.61 / 3600) * kDegToRad;
    pointing::hourAngleToHorizon(64.352133 * kDegToRad, dec, lat, az, el);
    CHECK_NEAR(az * kRadToDeg, 248.0337, 1e-4);
    CHECK_NEAR(el * kRadToDeg, 15.1249, 1e-4);

    // Parallactic angle: 0 on the meridian south of the zenith, negative to the east, positive to the west.
    CHECK_NEAR(pointing::parallacticAngle(0, 10 * kDegToRad, lat), 0, 1e-12);
    CHECK(pointing::parallacticAngle(-0.5, 10 * kDegToRad, lat) < 0);
    CHECK(pointing::parallacticAngle(0.5, 10 * kDegToRad, lat) > 0);

    // Polaris stands at about the latitude of the station whatever the time.
    const pointing::Site site = pointing::sfelSite();
    for (int hour : {0, 6, 12, 18})
    {
        const pointing::HorizonPosition p =
            pointing::toHorizon({37.95 * kDegToRad, 89.264 * kDegToRad}, {2026, 10, 28, hour, 0, 0}, site);
        CHECK_NEAR(p.elevation * kRadToDeg, 36.465, 0.8);
        CHECK(std::fabs(common::wrapPi(p.azimuth)) < 1.2 * kDegToRad);
    }

    // Camera model: x is +Az, y down is -El, turned by the roll.
    pointing::CameraModel cam{2, 0, false, 100, 50};
    pointing::MountOffset o = pointing::frameToMount(cam, 5, 0);
    CHECK_NEAR(o.dAzCosEl, 10, 1e-12);
    CHECK_NEAR(o.dEl, 0, 1e-12);
    o = pointing::frameToMount(cam, 0, 5);
    CHECK_NEAR(o.dEl, -10, 1e-12);
    cam.rollDeg = 90;
    o = pointing::frameToMount(cam, 5, 0);
    CHECK_NEAR(o.dAzCosEl, 0, 1e-12);
    CHECK_NEAR(o.dEl, -10, 1e-12);
    cam.rollDeg = 0;
    cam.mirrored = true;
    CHECK_NEAR(pointing::frameToMount(cam, 5, 0).dAzCosEl, -10, 1e-12);
    cam.mirrored = false;
    o = pointing::starOffset(cam, 103, 48);   // 3 px right of the axis, 2 px up
    CHECK_NEAR(o.dAzCosEl, 6, 1e-12);
    CHECK_NEAR(o.dEl, 4, 1e-12);

    // TPOINT pair: the encoders would read their value plus the star's offset with the star on the axis.
    const pointing::HorizonPosition star{1.0, 0.6, 0, 0};
    const pointing::TpointPair p = pointing::tpointPair(star, {1.001, 0.599}, {36, -18});
    CHECK_NEAR(p.demandedAz, 1.0, 1e-15);
    CHECK_NEAR(p.observedAz, 1.001 + 36 * kArcsecToRad / std::cos(0.599), 1e-15);
    CHECK_NEAR(p.observedEl, 0.599 - 18 * kArcsecToRad, 1e-15);
    return 0;
}
