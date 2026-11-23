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

/** @file wcs.cpp
 *  @brief Implementation of wcs.h.
 *  @author Degoras Project Team
 *  @copyright GPL-3.0-or-later
 */

// C++ INCLUDES
#include <initializer_list>
#include <cmath>

// PROJECT INCLUDES
#include "wcs/wcs.h"

// NAMESPACES
namespace dpot::wcs
{

namespace
{

// Sum of c[p][q] u^p v^q for p + q <= order.
double polynomial(const double (&c)[4][4], int order, double u, double v)
{
    double s = 0, up = 1;
    for (int p = 0; p <= order; p++, up *= u)
    {
        double vq = 1;
        for (int q = 0; p + q <= order; q++, vq *= v)
            s += c[p][q] * up * vq;
    }
    return s;
}

} // namespace

bool skyToFrame(const Wcs& w, const common::RaDec& sky, double& x, double& y)
{
    const double dra = sky.ra - w.ra0;
    const double cosc = std::sin(w.dec0) * std::sin(sky.dec) + std::cos(w.dec0) * std::cos(sky.dec) * std::cos(dra);
    if (cosc <= 0)
        return false;   // behind the tangent plane
    const double k = common::kRadToDeg / cosc;   // standard coordinates in degrees, as the CD matrix
    const double xi = k * std::cos(sky.dec) * std::sin(dra);
    const double eta =
        k * (std::cos(w.dec0) * std::sin(sky.dec) - std::sin(w.dec0) * std::cos(sky.dec) * std::cos(dra));
    const double det = w.cd11 * w.cd22 - w.cd12 * w.cd21;
    double u = (w.cd22 * xi - w.cd12 * eta) / det, v = (-w.cd21 * xi + w.cd11 * eta) / det;
    if (w.sip.valid)
    {
        const double du = polynomial(w.sip.ap, w.sip.apOrder, u, v), dv = polynomial(w.sip.bp, w.sip.apOrder, u, v);
        u += du;
        v += dv;
    }
    x = w.crpix1 + u - 1;          // FITS 1-based -> 0-based
    y = w.height - (w.crpix2 + v); // rows bottom up -> top down
    return true;
}

common::RaDec frameToSky(const Wcs& w, double x, double y)
{
    double u = x + 1 - w.crpix1, v = w.height - y - w.crpix2;
    if (w.sip.valid)
    {
        const double du = polynomial(w.sip.a, w.sip.aOrder, u, v), dv = polynomial(w.sip.b, w.sip.aOrder, u, v);
        u += du;
        v += dv;
    }
    const double xi = (w.cd11 * u + w.cd12 * v) * common::kDegToRad;
    const double eta = (w.cd21 * u + w.cd22 * v) * common::kDegToRad;
    const double den = std::cos(w.dec0) - eta * std::sin(w.dec0);
    const double ra = w.ra0 + std::atan2(xi, den);
    const double dec = std::atan2((std::sin(w.dec0) + eta * std::cos(w.dec0)) * std::cos(ra - w.ra0), den);
    return {common::wrapTwoPi(ra), dec};
}

double arcsecPerPixel(const Wcs& w)
{
    return std::sqrt(std::fabs(w.cd11 * w.cd22 - w.cd12 * w.cd21)) * 3600;
}

Wcs binnedToFrame(const Wcs& binned, int bin, int x0, int y0Top, int frameWidth, int frameHeight)
{
    Wcs out = binned;
    const double b = bin;
    const int y0Bottom = frameHeight - y0Top - binned.height * bin;   // the crop's offset counted from the bottom
    out.crpix1 = (binned.crpix1 - 1) * b + (b - 1) / 2 + 1 + x0;
    out.crpix2 = (binned.crpix2 - 1) * b + (b - 1) / 2 + 1 + y0Bottom;
    out.cd11 /= b;
    out.cd12 /= b;
    out.cd21 /= b;
    out.cd22 /= b;
    if (out.sip.valid)
        for (double (*t)[4] : {out.sip.a, out.sip.b, out.sip.ap, out.sip.bp})
            for (int p = 0; p < 4; p++)
                for (int q = 0; q < 4; q++)
                    t[p][q] *= std::pow(b, 1 - p - q);
    out.width = frameWidth;
    out.height = frameHeight;
    return out;
}

} // namespace dpot::wcs
