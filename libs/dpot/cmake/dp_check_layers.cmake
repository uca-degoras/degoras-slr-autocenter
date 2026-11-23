# **********************************************************************************************************************
#   dp_check_layers.cmake -- layer dependency rules of DegorasOpticalTracking (run by CTest as UT_LayerRules).
#   cmake -DDPOT_SOURCE_DIR=<project dir> -P dp_check_layers.cmake
#   The link step already keeps faster-astap and stb PRIVATE to the library. What it cannot see is an in-house
#   #include that crosses a layer the wrong way, or a faster-astap / stb header used outside the layers that wrap
#   them. This script checks exactly that:
#     - "<layer>/..." includes follow the table below;
#     - faster-astap ("astap/...") only in sources/solve and sources/measure: solve/solver.h is the one door to it;
#     - stb only in sources/io;
#     - imp/<name>.hpp (implementation) is included only by its own <name>.h.
#   Copyright (C) 2026 Degoras Project Team
#   SPDX-License-Identifier: GPL-3.0-or-later
# **********************************************************************************************************************

if(NOT DPOT_SOURCE_DIR)
    message(FATAL_ERROR "usage: cmake -DDPOT_SOURCE_DIR=... -P dp_check_layers.cmake")
endif()

# ----------------------------------------------------------------------------------------------------------------------
# RULES: area (folder relative to the project) -> in-house layers it may include (besides its own).

set(DPOT_LAYERS common io catalog detection wcs measure solve pointing)

set(DPOT_ALLOW_common    "common")
set(DPOT_ALLOW_io        "io;common")
set(DPOT_ALLOW_catalog   "catalog;common")
set(DPOT_ALLOW_detection "detection;common")
set(DPOT_ALLOW_wcs       "wcs;common")
set(DPOT_ALLOW_measure   "measure;detection;common")
set(DPOT_ALLOW_solve     "solve;detection;wcs;common")
set(DPOT_ALLOW_pointing  "pointing;common")
set(DPOT_ALLOW_apps      "${DPOT_LAYERS}")
set(DPOT_ALLOW_testing   "support;${DPOT_LAYERS}")

set(DPOT_AREAS)
foreach(_layer IN LISTS DPOT_LAYERS)
    list(APPEND DPOT_AREAS "includes/${_layer}" "sources/${_layer}")
endforeach()
list(APPEND DPOT_AREAS "sources/apps" "testing")

# ----------------------------------------------------------------------------------------------------------------------
# CHECK

set(_violations 0)
macro(dpot_violation file text)
    message(SEND_ERROR "${file}: ${text}")
    math(EXPR _violations "${_violations} + 1")
endmacro()

foreach(_area IN LISTS DPOT_AREAS)
    get_filename_component(_key ${_area} NAME)
    set(_allowed ${DPOT_ALLOW_${_key}})
    file(GLOB_RECURSE _files ${DPOT_SOURCE_DIR}/${_area}/*.h ${DPOT_SOURCE_DIR}/${_area}/*.hpp
                              ${DPOT_SOURCE_DIR}/${_area}/*.cpp)
    foreach(_file IN LISTS _files)
        file(RELATIVE_PATH _rel ${DPOT_SOURCE_DIR} ${_file})
        file(STRINGS ${_file} _includes REGEX "^[ \t]*#[ \t]*include")
        foreach(_line IN LISTS _includes)
            if(_line MATCHES "#[ \t]*include[ \t]*[\"<]astap/")
                if(NOT _rel MATCHES "^sources/(solve|measure)/")
                    dpot_violation(${_rel} "faster-astap is used only inside sources/solve and sources/measure")
                endif()
            elseif(_line MATCHES "#[ \t]*include[ \t]*[\"<]stb_")
                if(NOT _rel MATCHES "^sources/io/")
                    dpot_violation(${_rel} "stb is used only inside sources/io")
                endif()
            elseif(_line MATCHES "#[ \t]*include[ \t]*\"([a-z_]+)/([^\"]+)\"")   # in-house: quoted, <layer>/...
                set(_layer ${CMAKE_MATCH_1})
                set(_header ${CMAKE_MATCH_2})
                if(NOT _layer IN_LIST _allowed)
                    dpot_violation(${_rel} "includes layer '${_layer}' (allowed: ${_allowed})")
                endif()
                if(_header MATCHES "^imp/(.+)\\.hpp$")
                    set(_owner "includes/${_layer}/${CMAKE_MATCH_1}.h")
                    if(NOT _rel STREQUAL _owner)
                        dpot_violation(${_rel} "includes ${_header}; only ${_owner} may include it")
                    endif()
                endif()
            endif()
        endforeach()
    endforeach()
endforeach()

if(_violations GREATER 0)
    message(FATAL_ERROR "${_violations} layer rule violation(s)")
endif()
message(STATUS "Layer rules: OK")
