# **********************************************************************************************************************
#   DegorasOpticalTracking_PoF - Degoras Optical Tracking.
#
#   UT_* (unit, no data) and IT_* (integration, on the real images and the D80 star database under DPOT_DATA_DIR;
#   they exit 77, reported as skipped, when that data is missing). One executable per file, testing/<layer>/.
#
#   Copyright (C) 2026 Degoras Project Team
#
#   SPDX-License-Identifier: GPL-3.0-or-later
# **********************************************************************************************************************

file(GLOB_RECURSE DPOT_TEST_SOURCES CONFIGURE_DEPENDS
    ${CMAKE_CURRENT_SOURCE_DIR}/UT_*.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/IT_*.cpp)
file(GLOB DPOT_TEST_SUPPORT CONFIGURE_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/support/*.h)

foreach(_test_src IN LISTS DPOT_TEST_SOURCES)
    get_filename_component(_test_name ${_test_src} NAME_WE)
    add_executable(${_test_name} ${_test_src} ${DPOT_TEST_SUPPORT})
    target_include_directories(${_test_name} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
    target_link_libraries(${_test_name} PRIVATE DegorasOpticalTracking)
    target_compile_definitions(${_test_name} PRIVATE
        DPOT_SOURCE_DIR="${PROJECT_SOURCE_DIR}"
        DPOT_DATA_DIR="${DPOT_DATA_DIR}"
        DPOT_TEST_TMP_DIR="${CMAKE_CURRENT_BINARY_DIR}")
    dpot_target_warnings(${_test_name})

    add_test(NAME ${_test_name} COMMAND ${_test_name})
    set_tests_properties(${_test_name} PROPERTIES SKIP_RETURN_CODE 77)
    if(_test_name MATCHES "^IT_")
        # The first one to run builds the quad index (~4 s, 0.9 GB in DPOT_DATA_DIR/index); the lock keeps two from
        # writing it at once.
        set_tests_properties(${_test_name} PROPERTIES LABELS "integration" TIMEOUT 600 RESOURCE_LOCK dpot_index)
    else()
        set_tests_properties(${_test_name} PROPERTIES LABELS "unit" TIMEOUT 120)
    endif()
endforeach()

add_test(NAME UT_LayerRules
    COMMAND ${CMAKE_COMMAND} -DDPOT_SOURCE_DIR=${PROJECT_SOURCE_DIR}
            -P ${PROJECT_SOURCE_DIR}/cmake/dp_check_layers.cmake)
set_tests_properties(UT_LayerRules PROPERTIES LABELS "unit")
