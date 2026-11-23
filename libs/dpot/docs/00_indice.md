# DegorasOpticalTracking_PoF · Documentación

Punto de partida para la detección en tiempo real de estrellas, satélites y el haz láser en las cámaras de la estación
SFEL. Es una prueba de concepto (PoF): reúne en una librería C++ limpia lo que el experimento de *plate solving* de 2026
demostró que funciona, para que trabajéis sobre ella con libertad.

- Proyecto CMake: `DegorasOpticalTracking_PoF`. Namespace: `dpot`. C++17.
- Toolchains: `clang64` y `ucrt64` del entorno D:, en Debug (`dp-deb`) y Release (`dp-rel`).
- Licencia: GPL-3.0-or-later, Degoras Project Team (`LICENSE`). faster-astap (MPL-2.0) y stb (dominio público) van en
  `thirdparty/` con sus licencias.

## Qué hay ya

- **Resolución de placa** en 12–17 ms por imagen del RASA 8 (6248×4176), sin pista de apuntado, y en ~0,1 s con pista.
  Por debajo está faster-astap, el port a C++ de ASTAP, detrás de una única cabecera (`solve/solver.h`) con la forma
  de la API de la futura librería propia, DegorasAstrometry.
- **Identificación y medida de estrellas FK5** donde la solución las predice, también las saturadas.
- **La estrella única de la cámara del receptor de SFEL** (0,076″/px): centroide de alta precisión, diafragma de campo
  (dónde cae el eje del telescopio) y un mapa de exceso de verde para buscar el haz de 532 nm.
- **Apuntado**: Az/El aproximados de una estrella, el modelo de cada cámara (escala y giro calibrados) y el desvío
  estrella–eje en los ejes de la montura, con el par para un modelo TPOINT.
- **Manchas conectadas** (`detection/blobs.h`) con su elongación: el punto de partida para las trazas de satélites.

## Orden de lectura

| Fichero | Para qué |
|---|---|
| [01_puesta_en_marcha.md](01_puesta_en_marcha.md) | Entorno, datos, compilar, pasar los tests y ejecutar las herramientas |
| [02_arquitectura.md](02_arquitectura.md) | Capas, convenciones y dónde cambiar o añadir cosas |
| [03_algoritmos_y_resultados.md](03_algoritmos_y_resultados.md) | Cómo funciona cada paso, qué precisión y velocidad da, y sus límites |
| [changelog.md](changelog.md) | Registro de cambios |

El objetivo es vuestro: no hay lista de requisitos. Lo que el experimento dejó medido y pendiente está en `03`.

## Contenido del paquete

| Carpeta | Qué es |
|---|---|
| `DegorasOpticalTracking_PoF/` | Este proyecto, con esta documentación en `docs/` y los catálogos FK5 e IAU en `data/catalogs/` |
| `DegorasOpticalTracking_PoF_docs_word/` | La misma documentación en Word |
| `images/` | Las imágenes reales de la noche del 2026-10-28: RASA 8 (4 objetivos) y receptor de SFEL (simultáneas). `frame_times.csv`: la hora de cada imagen al milisegundo |
| `databases/` | Vacía: las bases de estrellas de ASTAP se descargan aparte (`LEEME.txt` y `01`, sección 2) |
| `experimento/` | El experimento original: documentos de análisis (`docs/`), programas C++ y scripts Python (`experiment/`), banco de pruebas (`bench/`), resultados en texto e imágenes finales, corpus público, fuentes de ASTAP (`refs/`) y la CLI de ASTAP como oráculo (`tools/`) |

## Estructura del repositorio

```
DegorasOpticalTracking_PoF/
├── CMakeLists.txt, CMakePresets.json, CMakeUserPresets.json.example, LICENSE
├── cmake/dp_check_layers.cmake    reglas de capas (test UT_LayerRules)
├── data/catalogs/                 FK5 (CDS I/149A e I/175) y nombres IAU (IAU-CSN)
├── docs/                          esta documentación
├── includes/<capa>/*.h            cabeceras públicas, por capa
├── sources/<capa>/*.cpp           implementación
├── sources/apps/                  dpot_identify y dpot_spot
├── testing/<capa>/                UT_* (unitarios) e IT_* (con imágenes reales)
└── thirdparty/                    faster-astap 0.5.0 (fuente completa) y stb
```
