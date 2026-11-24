# Overlay port de SuperNOVAS 1.8.0 facilitado por Ángel Vera (ROA), 23/11/2026.
# Motivo: el port de la baseline (1.7.2) no compila con MinGW (MSYS2 no trae <threads.h>).
# El parche mingw-srwlock.patch hace que MinGW use la API de bloqueo de Windows (SRWLock).

vcpkg_from_github(
    OUT_SOURCE_PATH SOURCE_PATH
    REPO Sigmyne/SuperNOVAS
    REF "v${VERSION}"
    SHA512 03b63021c3e2bbfda59deffef3b66204709257313cfde5e30ff7385608b16c62f8fc04d3b80aa6a2edea904c1229419b30b4786193560d1940b441e8a42c5b9b
    HEAD_REF main
    PATCHES
        mingw-srwlock.patch
)

vcpkg_check_features(OUT_FEATURE_OPTIONS FEATURE_OPTIONS
    FEATURES
        cpp            ENABLE_CPP
        solsys-calceph ENABLE_CALCEPH
)

vcpkg_cmake_configure(
    SOURCE_PATH "${SOURCE_PATH}"
    OPTIONS
        -DBUILD_TESTING=OFF
        -DENABLE_CSPICE=OFF
        ${FEATURE_OPTIONS}
)

vcpkg_cmake_install()

vcpkg_cmake_config_fixup(CONFIG_PATH "lib/cmake/${PORT}")

set(debug_pc "${CURRENT_PACKAGES_DIR}/debug/lib/pkgconfig/supernovas.pc")
if(EXISTS "${debug_pc}")
    vcpkg_replace_string("${debug_pc}" "-lsupernovas " "-lsupernovasd ")
    if("cpp" IN_LIST FEATURES)
        vcpkg_replace_string("${debug_pc}" "-lsupernovas++ " "-lsupernovas++d ")
    endif()
    if("solsys-calceph" IN_LIST FEATURES)
        vcpkg_replace_string("${debug_pc}" "-lsolsys-calceph " "-lsolsys-calcephd ")
    endif()
endif()

vcpkg_fixup_pkgconfig()

file(REMOVE_RECURSE "${CURRENT_PACKAGES_DIR}/share/doc")
file(REMOVE_RECURSE "${CURRENT_PACKAGES_DIR}/debug/include")
file(REMOVE_RECURSE "${CURRENT_PACKAGES_DIR}/debug/share")

# SOFA is intentionally not cited: SuperNOVAS describes this as a modified implementation
# that is neither provided nor endorsed by SOFA.
# https://github.com/Sigmyne/SuperNOVAS/blob/eafc3120cf257d643bd0676573420141ce8e53a6/src/c99/refract.c#L411-L423
vcpkg_install_copyright(FILE_LIST "${SOURCE_PATH}/LICENSE")
