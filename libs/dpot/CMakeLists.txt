# **********************************************************************************************************************
#   DegorasOpticalTracking_PoF - Degoras Optical Tracking.
#
#   Developed as free software by and for the Spanish Navy Observatory SLR station (SFEL) in San Fernando.
#
#   Copyright (C) 2026 Degoras Project Team
#
#   SPDX-License-Identifier: GPL-3.0-or-later
# **********************************************************************************************************************

# ----------------------------------------------------------------------------------------------------------------------
# INITIALIZATION

cmake_minimum_required(VERSION 3.31)

project(DegorasOpticalTracking_PoF VERSION 0.1.0 DESCRIPTION "Degoras Optical Tracking (PoF)" LANGUAGES CXX)

# ----------------------------------------------------------------------------------------------------------------------
# OPTIONS

option(DPOT_BUILD_TESTING "Build the UT_*, IT_* and FA_* tests." ON)

# The package root: images/, databases/d80 and index/ hang from it. The IT_* tests read their data there and skip
# themselves (exit code 77) when it is missing.
set(DPOT_DATA_DIR "${PROJECT_SOURCE_DIR}/.." CACHE PATH "Folder holding images/, databases/ and index/")

# ----------------------------------------------------------------------------------------------------------------------
# BASIC PROJECT CONFIGURATION

# C++17, as faster-astap and the Degoras libraries: this code is meant to end up in one of them.
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Every executable and test lands in bin/.
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

if(DPOT_BUILD_TESTING)
    enable_testing()
endif()

# ----------------------------------------------------------------------------------------------------------------------
# THIRD PARTY (vendored: faster-astap and the stb headers)

add_subdirectory(thirdparty)

# ----------------------------------------------------------------------------------------------------------------------
# HELPERS

# Warnings as PRIVATE per-target flags, for our own code only.
function(dpot_target_warnings target)
    target_compile_options(${target} PRIVATE $<$<CXX_COMPILER_ID:GNU,Clang,AppleClang>:-Wall;-Wextra;-Wpedantic>)
endfunction()

# Every header and source of one layer. Headers are listed so Qt Creator shows them.
function(dpot_glob_layer out_var layer)
    file(GLOB_RECURSE _files CONFIGURE_DEPENDS
        ${PROJECT_SOURCE_DIR}/includes/${layer}/*.h
        ${PROJECT_SOURCE_DIR}/includes/${layer}/*.hpp
        ${PROJECT_SOURCE_DIR}/sources/${layer}/*.cpp)
    set(${out_var} ${_files} PARENT_SCOPE)
endfunction()

# ----------------------------------------------------------------------------------------------------------------------
# LIBRARY

# The layers, from the bottom up; the allowed includes between them are in cmake/dp_check_layers.cmake.
set(DPOT_LAYERS common io catalog detection wcs measure solve pointing)
set(DPOT_FILES)
foreach(_layer IN LISTS DPOT_LAYERS)
    dpot_glob_layer(_layer_files ${_layer})
    list(APPEND DPOT_FILES ${_layer_files})
endforeach()

add_library(DegorasOpticalTracking STATIC ${DPOT_FILES})
target_include_directories(DegorasOpticalTracking PUBLIC ${PROJECT_SOURCE_DIR}/includes)
target_compile_definitions(DegorasOpticalTracking PUBLIC DPOT_VERSION_STRING="${PROJECT_VERSION}")
# faster-astap and stb are PRIVATE: no public header includes them, so whatever links the library cannot reach
# faster-astap except through solve/solver.h. That is the seam where DegorasAstrometry will replace it.
target_link_libraries(DegorasOpticalTracking
    PUBLIC  Threads::Threads
    PRIVATE faster_astap stb)
dpot_target_warnings(DegorasOpticalTracking)

# ----------------------------------------------------------------------------------------------------------------------
# APPLICATIONS

foreach(_app dpot_identify dpot_spot)
    add_executable(${_app} ${PROJECT_SOURCE_DIR}/sources/apps/${_app}/main.cpp)
    target_link_libraries(${_app} PRIVATE DegorasOpticalTracking)
    dpot_target_warnings(${_app})
endforeach()

# ----------------------------------------------------------------------------------------------------------------------
# TESTS

if(DPOT_BUILD_TESTING)
    add_subdirectory(testing)
endif()

# **********************************************************************************************************************
