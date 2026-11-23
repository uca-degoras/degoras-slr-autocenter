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

/** @file wcs.h
 *  @brief A plate solution: the gnomonic (TAN) projection, CD matrix and SIP distortion that map the sky to a frame.
 *
 *  The fields follow the FITS WCS standard as the solvers give them (CRPIX 1-based with rows bottom up, CD in degrees
 *  per pixel), so a Wcs can be written to a FITS header as is. The functions take and give frame pixels (0-based, y
 *  down, see common/frame.h).
 *  @author Degoras Project Team
 *  @copyright GPL-3.0-or-later
 */

#pragma once

// PROJECT INCLUDES
#include "common/angles.h"

// NAMESPACES
namespace dpot::wcs
{

// ---------------------------------------------------------------------------------------------------------------------
// TYPES

/// SIP distortion: a/b map pixel offsets to intermediate offsets (A_p_q = a[p][q]), ap/bp the reverse.
struct Sip
{
    bool valid = false;
    int aOrder = 0;
    int apOrder = 0;
    double a[4][4] = {};
    double b[4][4] = {};
    double ap[4][4] = {};
    double bp[4][4] = {};
};

struct Wcs
{
    double ra0 = 0;          ///< CRVAL1, rad
    double dec0 = 0;         ///< CRVAL2, rad
    double crpix1 = 0;       ///< FITS: 1-based
    double crpix2 = 0;       ///< FITS: 1-based, rows counted from the bottom
    double cd11 = 0, cd12 = 0, cd21 = 0, cd22 = 0;   ///< deg / px
    double rotationDeg = 0;  ///< The solver's CROTA2: the angle of north in the image
    Sip sip;
    int width = 0;           ///< Of the frame the solution belongs to
    int height = 0;
};

// ---------------------------------------------------------------------------------------------------------------------
// FUNCTIONS

/// Sky (rad) -> frame pixel, through the inverse SIP when there is one. False behind the tangent plane.
bool skyToFrame(const Wcs& wcs, const common::RaDec& sky, double& x, double& y);

/// Frame pixel -> sky (rad), through the forward SIP when there is one.
common::RaDec frameToSky(const Wcs& wcs, double x, double y);

/// Plate scale: sqrt |det CD|, arcsec per pixel.
double arcsecPerPixel(const Wcs& wcs);

/// The solution of a binned crop (binFrame) rewritten for the whole frame: the reference pixel moved and scaled, CD
/// divided by `bin`, each SIP term c u^p v^q times bin^(1-p-q). x0 / y0Top: the crop's left column and top row in the
/// frame (BinnedGeometry); frameWidth / frameHeight: the frame's size.
Wcs binnedToFrame(const Wcs& binned, int bin, int x0, int y0Top, int frameWidth, int frameHeight);

} // namespace dpot::wcs
