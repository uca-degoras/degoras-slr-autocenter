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

/** @file main.cpp
 *  @brief dpot_spot: the one bright star on raw Bayer frames of the SFEL receiver camera (ASI585MC Pro, RAW8).
 *
 *    dpot_spot [--out dir [--show name]] <raw Bayer png>...
 *
 *  Each 2x2 Bayer cell is summed into one grey pixel, so the colour pattern does not pull the centroid, and the star is
 *  measured twice (measure/spot_measure.h): "win", the Gaussian-windowed centroid, and "cog", the centre of gravity.
 *  Per frame: both centres (raw pixels, frame convention), peak, raw pixels at 255, flux, FWHM, and how many sources
 *  stand 10 sigma out. Then the mean, standard deviation and drift per frame of "win", and the star's offset from the
 *  image centre in the mount's axes (pointing/camera_model.h, sfelReceiverCamera).
 *
 *  With --out, for the run (named after the PNGs' two parent folders):
 *    <name>_crop.png    the frame nearest the median position (or --show <file name>) 4x around the star: detection
 *                       outline (green), saturated pixels (red), both centres, the window;
 *    <name>_green.png   the green excess of the run's mean (detection/green_excess.h), +-5 sigma, the star masked: the
 *                       532 nm laser beam would light only the green pixels;
 *  and prints the field stop's centre (detection/field_stop.h), where the telescope axis falls.
 *  Port of the experiment's spot (without its search for faint field stars).
 *  @author Degoras Project Team
 *  @copyright GPL-3.0-or-later
 */

// C++ INCLUDES
#include <initializer_list>
#include <algorithm>
#include <cmath>
#include <cstdio>
#include <filesystem>
#include <string>
#include <vector>

// PROJECT INCLUDES
#include "common/stats.h"
#include "detection/bayer.h"
#include "detection/field_stop.h"
#include "detection/green_excess.h"
#include "io/canvas.h"
#include "io/png_io.h"
#include "measure/spot_measure.h"
#include "pointing/camera_model.h"

using namespace dpot;

namespace
{

// The frame 4x around the star, grey pixels stretched; the detection (3 sigma) in green, saturated raw pixels in red,
// the windowed centroid in magenta with its 4 sigma window in orange, the centre of gravity in cyan.
void writeCrop(const common::Frame& raw, const common::FloatImage& g, const measure::SpotMeasurement& m,
               const std::string& title, const std::string& path)
{
    constexpr int kHalf = 60, kZoom = 4;
    const io::Canvas grey = io::stretched(g, m.background, m.noise);
    io::Canvas crop = grey.crop(m.winX, m.winY, kHalf, kZoom);
    auto toCrop = [&](double v, double c) { return (v - (std::lround(c) - kHalf) + 0.5) * kZoom; };
    for (int y = static_cast<int>(m.winY) - kHalf; y < static_cast<int>(m.winY) + kHalf; y++)
        for (int x = static_cast<int>(m.winX) - kHalf; x < static_cast<int>(m.winX) + kHalf; x++)
        {
            if (x < 1 || y < 1 || x >= g.width - 1 || y >= g.height - 1)
                continue;
            auto dark = [&](int i, int j) { return g.at(i, j) - m.background < 3 * m.noise; };
            const bool edge = !dark(x, y) && (dark(x - 1, y) || dark(x + 1, y) || dark(x, y - 1) || dark(x, y + 1));
            if (edge)
                crop.dot(std::lround(toCrop(x, m.winX)), std::lround(toCrop(y, m.winY)), io::colors::kGreen);
            if (measure::countSaturated(raw, 2 * x, 2 * y, 1) > 0)
                crop.circle(toCrop(x, m.winX), toCrop(y, m.winY), 1, 1, io::colors::kRed);
        }
    crop.circle(toCrop(m.winX, m.winX) - 0.5 * kZoom, toCrop(m.winY, m.winY) - 0.5 * kZoom, 4 * m.sigma * kZoom, 1,
                io::colors::kOrange);
    crop.cross(toCrop(m.winX, m.winX) - 0.5 * kZoom, toCrop(m.winY, m.winY) - 0.5 * kZoom, 12, 0, io::colors::kMagenta);
    crop.cross(toCrop(m.cogX, m.winX) - 0.5 * kZoom, toCrop(m.cogY, m.winY) - 0.5 * kZoom, 8, 3, io::colors::kCyan);
    crop.text(10, 10, title, 2, io::colors::kWhite);
    crop.savePng(path);
}

} // namespace

int main(int argc, char** argv)
{
    std::string outDir, show;
    int a = 1;
    for (; a + 1 < argc && argv[a][0] == '-'; a += 2)
    {
        if (std::string(argv[a]) == "--out")
            outDir = argv[a + 1];
        else if (std::string(argv[a]) == "--show")
            show = argv[a + 1];
    }
    if (a >= argc)
    {
        std::fprintf(stderr, "usage: dpot_spot [--out dir [--show name]] <raw Bayer png>...\n");
        return 2;
    }
    std::printf("file  win_x win_y  cog_x cog_y  peak sat255  flux  fwhm_px  sources\n");
    std::vector<double> wx, wy, fwhm;
    std::vector<int> okFrame;
    detection::ColourPlanes sum;   // the run's sum of colour planes, for --out
    int planes = 0;
    for (int i = a; i < argc; i++)
    {
        common::Frame raw;
        std::string error;
        if (!io::loadFrame(argv[i], raw, &error))
        {
            std::printf("%s\n", error.c_str());
            continue;
        }
        if (!outDir.empty())
        {
            const detection::ColourPlanes p = detection::splitRggb(raw);
            if (planes++ == 0)
                sum = p;
            else
                for (std::size_t k = 0; k < p.g.px.size(); k++)
                {
                    sum.r.px[k] += p.r.px[k];
                    sum.g.px[k] += p.g.px[k];
                    sum.b.px[k] += p.b.px[k];
                }
        }
        const common::FloatImage g = detection::bayerSum(raw);
        const measure::SpotMeasurement m = measure::measureSpot(g);
        const int sat = measure::countSaturated(raw, detection::greyToRaw(m.seedX), detection::greyToRaw(m.seedY), 100);
        std::printf("%s %8.2f %8.2f  %8.2f %8.2f  %4.0f %5d %8.0f %6.2f  %3zu\n",
                    std::filesystem::path(argv[i]).filename().string().c_str(), detection::greyToRaw(m.winX),
                    detection::greyToRaw(m.winY), detection::greyToRaw(m.cogX), detection::greyToRaw(m.cogY), m.peak,
                    sat, m.flux, 2 * 2.3548 * m.sigma, m.sources.size());
        if (m.found())
        {
            wx.push_back(detection::greyToRaw(m.winX));
            wy.push_back(detection::greyToRaw(m.winY));
            fwhm.push_back(2 * 2.3548 * m.sigma);
            okFrame.push_back(i);
        }
    }
    std::printf("== %zu frames with the star: win x %.2f +- %.2f, y %.2f +- %.2f raw px; drift %.4f, %.4f px/frame\n",
                wx.size(), common::mean(wx), common::stdev(wx), common::mean(wy), common::stdev(wy), common::slope(wx),
                common::slope(wy));
    if (wx.empty())
        return 1;
    const pointing::CameraModel camera = pointing::sfelReceiverCamera();
    const pointing::MountOffset o = pointing::starOffset(camera, common::mean(wx), common::mean(wy));
    std::printf("star - image centre: dAz*cosEl %+.1f\", dEl %+.1f\" (%.4f\"/px, roll %+.1f deg)\n", o.dAzCosEl, o.dEl,
                camera.arcsecPerPixel, camera.rollDeg);
    if (outDir.empty())
        return 0;

    // The run's name from the two parent folders, e.g. MATAR_2026-10-28_19_24_41Z.
    const std::filesystem::path first(argv[a]);
    const std::string label = first.parent_path().parent_path().filename().string() + "_" +
                              first.parent_path().filename().string();
    std::filesystem::create_directories(outDir);
    const std::string base = outDir + "/" + label;

    // The frame nearest the median position, or the one --show names.
    const double mx = common::median(std::vector<float>(wx.begin(), wx.end()));
    const double my = common::median(std::vector<float>(wy.begin(), wy.end()));
    std::size_t rep = 0;
    for (std::size_t k = 1; k < wx.size(); k++)
        if (std::hypot(wx[k] - mx, wy[k] - my) < std::hypot(wx[rep] - mx, wy[rep] - my))
            rep = k;
    for (std::size_t k = 0; k < okFrame.size() && !show.empty(); k++)
        if (std::filesystem::path(argv[okFrame[k]]).filename().string() == show)
            rep = k;
    common::Frame raw;
    io::loadFrame(argv[okFrame[rep]], raw);
    const common::FloatImage g = detection::bayerSum(raw);
    const measure::SpotMeasurement m = measure::measureSpot(g);
    const std::string repName = std::filesystem::path(argv[okFrame[rep]]).filename().string();
    writeCrop(raw, g, m, label + "\n" + repName, base + "_crop.png");

    // The run's mean planes: the field stop (where the axis falls) and the green excess.
    for (common::FloatImage* p : {&sum.r, &sum.g, &sum.b})
        for (float& v : p->px)
            v /= static_cast<float>(planes);
    const detection::FieldMask mask = detection::fieldMask(sum.g);
    double fcx = 0, fcy = 0, fr = 0;
    if (detection::fitFieldStop(mask, sum.g.width, sum.g.height, fcx, fcy, fr))
    {
        const double sx = detection::greyToRaw(fcx), sy = detection::greyToRaw(fcy);
        std::printf("field stop: centre %.1f, %.1f raw px (%+.1f, %+.1f from the image centre), radius %.1f raw px\n",
                    sx, sy, sx - (raw.width - 1) / 2.0, sy - (raw.height - 1) / 2.0, 2 * fr);
    }
    double k = 0;
    const common::FloatImage e = detection::greenExcess(sum, mask, detection::rawToGrey(common::mean(wx)),
                                                        detection::rawToGrey(common::mean(wy)),
                                                        3 * common::mean(fwhm) / 2, k);
    std::vector<float> s;
    for (std::size_t i = 0; i < e.px.size(); i += 3)
        if (mask.inner[i])
            s.push_back(e.px[i]);
    const float sd = std::max(common::madSigma(s, 0), 1e-6f);
    io::Canvas map(e.width, e.height);
    for (std::size_t i = 0; i < e.px.size(); i++)
    {
        const float v = mask.inner[i] ? std::clamp(e.px[i] / (5 * sd), -1.0f, 1.0f) : 0;
        std::fill_n(&map.rgb()[i * 3], 3, static_cast<unsigned char>(std::lround(128 + 127 * v)));
    }
    map.circle(detection::rawToGrey(common::mean(wx)), detection::rawToGrey(common::mean(wy)),
               3 * common::mean(fwhm) / 2, 2, io::colors::kRed);
    char text[200];
    std::snprintf(text, sizeof text, "%s: green excess G - %.3f (R+B)/2 of the mean of %d frames, +-5 sigma\n"
                                     "(the 532 nm beam would light only the green pixels; red: star masked)",
                  label.c_str(), k, planes);
    map.text(20, 20, text, 2, io::colors::kYellow);
    map.savePng(base + "_green.png");
    std::printf("wrote %s_crop.png and %s_green.png (frame shown: %s)\n", base.c_str(), base.c_str(), repName.c_str());
    return 0;
}
