# 02 · Arquitectura

## 1. Capas

Una librería estática, `DegorasOpticalTracking`, y dos programas. Cada capa tiene sus cabeceras en `includes/<capa>/` y
su código en `sources/<capa>/`; los includes son planos: `#include "detection/star_detector.h"`.

| Capa | Qué tiene | Puede incluir |
|---|---|---|
| `common` | `Frame` y `FloatImage`, ángulos y `RaDec`, estadística robusta, `parallelFor`, `UtcTime` y fechas julianas | — |
| `io` | PNG (stb), `Canvas` para dibujar resultados, la hora de captura de cada imagen | `common` |
| `catalog` | FK5 (I/149A + I/175) con movimiento propio, nombres propios IAU | `common` |
| `detection` | Fondo por celdas, detector rápido de estrellas, binning, Bayer, manchas conectadas, diafragma, exceso de verde | `common` |
| `wcs` | La solución de placa (`Wcs`: TAN + CD + SIP), cielo ↔ píxel | `common` |
| `measure` | Medida de una estrella donde la solución la predice; la estrella única de SFEL | `detection`, `common` |
| `solve` | **La costura con faster-astap** (`solver.h`) | `detection`, `wcs`, `common` |
| `pointing` | Az/El, ángulo paraláctico, modelo de cámara, desvío estrella–eje, par TPOINT | `common` |
| `apps` | `dpot_identify`, `dpot_spot` | todas |

`UT_LayerRules` (`cmake/dp_check_layers.cmake`) comprueba la tabla en cada `ctest`, y además que faster-astap
(`astap/...`) solo se incluya en `sources/solve` y `sources/measure`, y stb solo en `sources/io`.

## 2. La costura: `solve/solver.h`

faster-astap es el puente hasta que exista DegorasAstrometry, nuestra librería de *plate solving* (su diseño está en
`experimento/docs/14_arquitectura.md`). Para que cambiarla no toque nada más:

- `solver.h` no muestra ningún tipo de faster-astap (*pimpl*), y la librería lo enlaza en privado.
- Sus nombres son los de la API prevista para DegorasAstrometry: `Status` con los códigos de salida de ASTAP,
  `SolveRequest` con posición y radio (180 = a ciegas), `SolveResult` con la `Wcs`.
- Fuera de `sources/solve` solo `sources/measure` usa faster-astap, para su `hfd()`.

Si necesitáis algo más de faster-astap, ampliad `solver.h` (o añadid otra cabecera en `solve`) en vez de incluirlo en
otra capa.

faster-astap va **como fuente completa** en `thirdparty/faster-astap` (versión 0.5.0, commit `a0b4baf`, MPL-2.0), con un
solo parche (`thirdparty/patches/`): podéis leerlo, cambiarlo y recompilarlo. `thirdparty/CMakeLists.txt` compila sus
dos librerías y cuatro de sus tests (`FA_*`); su `CMakeLists.txt` original sigue ahí para compilarlo aparte (CLI, Python,
N.I.N.A.). Si cambiáis un fichero suyo, conservad la cabecera MPL y anotadlo.

## 3. Convenciones

- **Píxeles:** en todo el proyecto, píxeles de la imagen tal como la da la cámara: base 0, x a la derecha, **y hacia
  abajo**, el centro del píxel de arriba a la izquierda en (0, 0). Solo `solve` y `measure` pasan al convenio FITS de
  faster-astap (filas de abajo arriba, base 1) y vuelven.
- **Muestras:** `Frame` guarda una muestra de 16 bits por píxel; los datos de 8 bits vienen multiplicados por 257 (255 →
  65535). `Frame::raw()` da el valor original.
- **Ángulos:** radianes, salvo que el nombre diga `Deg` o `Arcsec`. Posiciones J2000.
- **Hora:** UTC (`UtcTime`). La de cada imagen sale de `io::captureTime` (ver su cabecera).
- **Estilo:** el de Degoras, como en el proyecto de bases de datos: C++17, cabeceras `.h`, Allman, 120 columnas, sin
  excepciones en la frontera de `solve`, comentarios en inglés y documentación en español.

## 4. Tests

Un ejecutable por fichero `testing/<capa>/UT_*.cpp` o `IT_*.cpp`, con `CHECK` y `CHECK_NEAR` de
`testing/support/check.h` (activos también en Release). Un test sin sus datos sale con 77 y CTest lo cuenta como
*Skipped* (`testing::requireData`). Los `UT_*` usan imágenes sintéticas o valores publicados (los ejemplos de Meeus para
el tiempo sidéreo, la precesión y Az/El); los `IT_*`, las imágenes reales y la D80 de `DPOT_DATA_DIR`.

## 5. Por dónde crecer

- **Tiempo real:** los programas leen ficheros. Una cámara en directo (SDK de ZWO, GStreamer o Qt Multimedia, todos en
  el entorno) solo tiene que llenar un `common::Frame`. El presupuesto por imagen está en `03`; `Solver` admite una
  resolución a la vez, así que un hilo por cámara, cada uno con su `Solver`.
- **Satélites:** `detection::findBlobs` da las manchas con su elongación y dirección; una traza larga y con dirección
  constante entre imágenes es un satélite. La `Wcs` convierte sus extremos a RA/Dec (`wcs::frameToSky`).
- **Haz láser:** `detection::greenExcess` está listo pero sin probar con el haz: faltan imágenes con el láser.
- **Apuntado de precisión:** `pointing/horizon.h` deja fuera nutación, aberración y refracción (ver su cabecera). Para
  modelos de apuntado hace falta el lugar aparente y observado completo; la decisión para DegorasAstrometry es SuperNOVAS.
- **Una capa o un módulo nuevo:** carpeta en `includes/` y `sources/`, su fila en la tabla de
  `cmake/dp_check_layers.cmake` y en `DPOT_LAYERS` de `CMakeLists.txt`, y sus tests en `testing/<capa>/`.
