/***********************************************************************************************************************
 *  DegorasOpticalTracking_PoF - Degoras Optical Tracking: real-time detection of stars, satellites and the laser
 *  beam in the cameras of the SFEL station.
 *
 *  Copyright (C) 2026 Degoras Project Team
 *
 *  SPDX-License-Identifier: GPL-3.0-or-later
 **********************************************************************************************************************/

/** @file IT_SpotSfel.cpp
 *  @brief Integration test on real SFEL receiver frames (DPOT_DATA_DIR/images/SFEL-RX600_ASI585MCPRO/MATAR): the star
 *         is found on every frame and stays put (the mount tracks), and the field stop of the mean green plane lies
 *         where the experiment found it (raw px ~1965-1972, 1120-1160). Skipped (77) without the images.
 */

// C++ INCLUDES
#include <algorithm>
#include <cstdio>
#include <filesystem>
#include <string>
#include <vector>

// PROJECT INCLUDES
#include "common/stats.h"
#include "detection/bayer.h"
#include "detection/field_stop.h"
#include "io/png_io.h"
#include "measure/spot_measure.h"
#include "support/check.h"

using namespace dpot;
namespace fs = std::filesystem;

int main()
{
    const fs::path dir = fs::path(DPOT_DATA_DIR) / "images" / "SFEL-RX600_ASI585MCPRO" / "MATAR";
    testing::requireData(dir.string(), "SFEL images");
    std::vector<std::string> pngs;
    for (const auto& e : fs::recursive_directory_iterator(dir))
        if (e.path().extension() == ".PNG" || e.path().extension() == ".png")
            pngs.push_back(e.path().string());
    std::sort(pngs.begin(), pngs.end());
    CHECK(pngs.size() >= 10);
    pngs.resize(10);

    std::vector<double> wx, wy;
    common::FloatImage green;
    for (const std::string& png : pngs)
    {
        common::Frame raw;
        CHECK(io::loadFrame(png, raw));
        CHECK(raw.width == 3840 && raw.height == 2160 && raw.bitDepth == 8);
        const measure::SpotMeasurement m = measure::measureSpot(detection::bayerSum(raw));
        CHECK(m.found());
        wx.push_back(detection::greyToRaw(m.winX));
        wy.push_back(detection::greyToRaw(m.winY));
        const detection::ColourPlanes p = detection::splitRggb(raw);
        if (green.px.empty())
            green = p.g;
        else
            for (std::size_t k = 0; k < green.px.size(); k++)
                green.px[k] += p.g.px[k];
    }
    std::printf("star at %.2f +- %.2f, %.2f +- %.2f raw px\n", common::mean(wx), common::stdev(wx), common::mean(wy),
                common::stdev(wy));
    CHECK(common::stdev(wx) < 5 && common::stdev(wy) < 5);

    const detection::FieldMask mask = detection::fieldMask(green);
    double cx = 0, cy = 0, r = 0;
    CHECK(detection::fitFieldStop(mask, green.width, green.height, cx, cy, r));
    std::printf("field stop centre %.1f, %.1f raw px, radius %.1f raw px\n", detection::greyToRaw(cx),
                detection::greyToRaw(cy), 2 * r);
    CHECK_NEAR(detection::greyToRaw(cx), 1968, 30);
    CHECK_NEAR(detection::greyToRaw(cy), 1140, 40);
    return 0;
}
