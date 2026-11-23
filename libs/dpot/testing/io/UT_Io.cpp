/***********************************************************************************************************************
 *  DegorasOpticalTracking_PoF - Degoras Optical Tracking: real-time detection of stars, satellites and the laser
 *  beam in the cameras of the SFEL station.
 *
 *  Copyright (C) 2026 Degoras Project Team
 *
 *  SPDX-License-Identifier: GPL-3.0-or-later
 **********************************************************************************************************************/

/** @file UT_Io.cpp
 *  @brief Unit test of the io layer: PNG round trip, drawing, and the three sources of a frame's capture time.
 */

// C++ INCLUDES
#include <filesystem>
#include <fstream>

// PROJECT INCLUDES
#include "io/canvas.h"
#include "io/capture_time.h"
#include "io/png_io.h"
#include "support/check.h"

using namespace dpot;
namespace fs = std::filesystem;

int main()
{
    const fs::path tmp = fs::path(DPOT_TEST_TMP_DIR) / "UT_Io";
    fs::remove_all(tmp);
    fs::create_directories(tmp);

    // An 8 bit frame survives a round trip, and comes back on the 16 bit scale.
    common::Frame f;
    f.width = 37;
    f.height = 23;
    f.bitDepth = 8;
    for (int i = 0; i < f.width * f.height; i++)
        f.px.push_back(static_cast<std::uint16_t>((i * 7 % 256) * 257));
    const std::string png = (tmp / "frame.png").string();
    CHECK(io::saveFrame8(png, f));
    common::Frame g;
    std::string error;
    CHECK(io::loadFrame(png, g, &error));
    CHECK(g.width == 37 && g.height == 23 && g.bitDepth == 8 && g.px == f.px);
    CHECK(g.raw(5, 0) == 35);
    CHECK(!io::loadFrame((tmp / "missing.png").string(), g, &error) && !error.empty());

    // Drawing stays inside the canvas; text marks pixels; crops have the asked size.
    io::Canvas c(100, 60);
    c.circle(50, 30, 200, 1, io::colors::kRed);   // entirely outside: no crash
    c.text(5, 5, "FK5 745\nAltair", 2, io::colors::kWhite);
    int lit = 0;
    for (unsigned char v : c.rgb())
        lit += v != 0;
    CHECK(lit > 50);
    CHECK(io::Canvas::textWidth("FK5", 2) > 0);
    const io::Canvas z = c.crop(10, 10, 8, 3);
    CHECK(z.width() == 48 && z.height() == 48);
    CHECK(c.savePng((tmp / "canvas.png").string()) && c.saveJpg((tmp / "canvas.jpg").string()));

    // Capture time: file name (minute), session folder (second), frame_times.csv (millisecond).
    common::UtcTime t;
    CHECK(io::captureTime((tmp / "2026-10-28-1930_0-CapObj_0000.PNG").string(), t) == io::TimeSource::FILE_NAME);
    CHECK(t.hour == 19 && t.minute == 30 && t.second == 0);
    const fs::path session = tmp / "images" / "MATAR" / "2026-10-28_19_24_41Z";
    fs::create_directories(session);
    const fs::path frame = session / "2026-10-28-1924_4-CapObj_0033.PNG";
    CHECK(io::captureTime(frame.string(), t) == io::TimeSource::SESSION);
    CHECK(t.hour == 19 && t.minute == 24 && t.second == 41);
    std::ofstream(tmp / "images" / "frame_times.csv")
        << "path,utc\nMATAR/2026-10-28_19_24_41Z/2026-10-28-1924_4-CapObj_0033.PNG,2026-10-28T19:24:52.125Z\n";
    CHECK(io::captureTime(frame.string(), t) == io::TimeSource::FRAME_TIMES);
    CHECK(t.minute == 24 && t.second == 52.125);
    CHECK(io::captureTime((tmp / "no_date.png").string(), t) == io::TimeSource::NONE);
    return 0;
}
