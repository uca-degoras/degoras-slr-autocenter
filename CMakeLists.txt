cmake_minimum_required(VERSION 3.31)

project(degoras_slr_autocenter LANGUAGES CXX)

# Raíz de las carpetas databases/, images/ e index/ que leen los tests IT_* de dpot (están en .gitignore).
# Si faltan, esos tests salen como Skipped. Se fija antes de add_subdirectory para que dpot no use su valor por defecto.
set(DPOT_DATA_DIR "${CMAKE_CURRENT_SOURCE_DIR}" CACHE PATH "Carpeta con databases/, images/ e index/")

# Necesario en la raíz: sin él, ctest ejecutado desde el árbol de compilación raíz no ve los tests de dpot.
enable_testing()

# Librería del ROA, sin modificar. Define sus tests (UT_*, IT_*, FA_*) con add_test.
add_subdirectory(libs/dpot)
