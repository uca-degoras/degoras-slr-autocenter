# Registro de cambios

Una entrada por entrega o cambio relevante, la más reciente arriba.

## 0.1.0 · 2026-11-16 · Entrega del PoF

Primera versión, a partir del experimento de *plate solving* de 2026 (`experimento/`).

- Proyecto CMake `DegorasOpticalTracking_PoF`, namespace `dpot`, C++17, con los presets del entorno D: (`clang64` y
  `ucrt64`, Debug y Release). Build en `$DEVSYSTEM_BUILDTREES/DegorasOpticalTracking_PoF/<toolchain>-dynamic-<config>`.
- faster-astap 0.5.0 (commit `a0b4baf`) como fuente en `thirdparty/`, con el parche de `<cstdlib>` para `clang64`; stb.
- Capas `common`, `io`, `catalog`, `detection`, `wcs`, `measure`, `solve` y `pointing`, con reglas comprobadas en
  `UT_LayerRules`. faster-astap solo detrás de `solve/solver.h`, con la forma de la API de DegorasAstrometry.
- Port de los algoritmos del experimento (`fk5_fast`, `spot`, `altaz_check.py`, `calib.py`): detector rápido sobre la
  imagen con binning, resolución a ciegas por índice y con pista en espiral, medida de FK5 (también saturadas), estrella
  única de SFEL, diafragma, exceso de verde, Az/El, modelo de cámara y par TPOINT. Mismos resultados que el experimento
  sobre las imágenes de Matar.
- Nuevo respecto al experimento: manchas conectadas con elongación (`findBlobs`) y la hora de cada imagen desde
  `frame_times.csv`.
- `dpot_identify` y `dpot_spot`.
- 15 tests: unitarios (con referencias de Meeus), cuatro de faster-astap, reglas de capas y dos de integración con las
  imágenes reales.
