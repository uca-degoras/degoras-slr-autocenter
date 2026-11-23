# **********************************************************************************************************************
#   DegorasOpticalTracking_PoF - Degoras Optical Tracking.
#
#   faster-astap 0.5.0 (https://github.com/Pop-Vlad/faster-astap, commit a0b4baf, MPL-2.0), the C++ port of ASTAP's
#   plate solver, vendored as source so it can be read, changed and rebuilt with the project. Only change:
#   patches/missing-cstdlib-include.patch (libc++ in clang64 no longer pulls <cstdlib> in transitively).
#   Its own CMakeLists.txt is kept untouched for building it on its own (CLI, Python, N.I.N.A.); this file builds just
#   the two libraries and four of its tests, with the image decoders that need outside libraries left out (FITS, Netpbm
#   and BMP need none).
#
#   Copyright (C) 2026 Degoras Project Team
#
#   SPDX-License-Identifier: GPL-3.0-or-later
# **********************************************************************************************************************

set(_fa ${CMAKE_CURRENT_SOURCE_DIR}/faster-astap)

# Pixels in memory -> solution: detection, star databases, quads, matching, both solvers.
add_library(faster_astap STATIC
    ${_fa}/src/astro_math.cpp
    ${_fa}/src/mapped_file.cpp
    ${_fa}/src/parallel.cpp
    ${_fa}/src/quad_batch.cpp
    ${_fa}/src/quad_index.cpp
    ${_fa}/src/index_solver.cpp
    ${_fa}/src/calc_trans_cubic.cpp
    ${_fa}/src/matching.cpp
    ${_fa}/src/quads.cpp
    ${_fa}/src/star_database.cpp
    ${_fa}/src/star_detection.cpp
    ${_fa}/src/solver.cpp
    ${_fa}/src/solve_service.cpp
    ${_fa}/src/spiral_service.cpp)
target_include_directories(faster_astap SYSTEM PUBLIC ${_fa}/include)
target_link_libraries(faster_astap PUBLIC Threads::Threads)
target_compile_definitions(faster_astap PUBLIC $<$<PLATFORM_ID:Windows>:NOMINMAX;WIN32_LEAN_AND_MEAN>)

# Image files -> pixels (FITS, FITS.fz Rice, Netpbm, BMP) and the .ini/.wcs front ends.
add_library(faster_astap_image STATIC
    ${_fa}/src/image/image_io.cpp
    ${_fa}/src/image/fits.cpp
    ${_fa}/src/image/rice.cpp
    ${_fa}/src/image/image_pnm.cpp
    ${_fa}/src/image/image_bmp.cpp
    ${_fa}/src/image/image_png.cpp
    ${_fa}/src/image/image_jpeg.cpp
    ${_fa}/src/image/image_tiff.cpp
    ${_fa}/src/image/image_raw.cpp
    ${_fa}/src/solve_service_files.cpp
    ${_fa}/src/spiral_service_files.cpp)
target_link_libraries(faster_astap_image PUBLIC faster_astap)

if(DPOT_BUILD_TESTING)
    # Upstream's own tests, named FA_* in CTest. They need no star database.
    foreach(_t solver_tests quad_batch_tests index_file_tests image_io_tests)
        add_executable(FA_${_t} ${_fa}/tests/${_t}.cpp)
        target_link_libraries(FA_${_t} PRIVATE faster_astap_image)
        target_compile_definitions(FA_${_t} PRIVATE
            ASTAP_TEST_DATA_DIR="${_fa}/tests/data"
            ASTAP_TEST_TMP_DIR="${CMAKE_CURRENT_BINARY_DIR}")
        add_test(NAME FA_${_t} COMMAND FA_${_t})
        set_tests_properties(FA_${_t} PROPERTIES LABELS "unit;faster-astap" TIMEOUT 300)
    endforeach()
endif()

# stb (public domain, https://github.com/nothings/stb, the copies vcpkg installed for the experiment): PNG reading and
# writing, and the bitmap font of the annotated images. Compiled once here; SYSTEM so their warnings stay out.
add_library(stb STATIC ${CMAKE_CURRENT_SOURCE_DIR}/stb/stb_impl.cpp)
target_include_directories(stb SYSTEM PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/stb)
