# degoras-slr-autocenter

Prueba de concepto de centrado automático por imagen CMOS para la Estación SLR del
Real Instituto y Observatorio de la Armada (ROA), San Fernando – Proyecto Degoras.
Grupo de estudiantes de la Universidad de Cádiz.

A partir de vídeo o secuencias FITS de la cámara del telescopio receptor, detecta el
satélite y la traza del haz láser, calcula su centroide subpíxel y obtiene el vector de
desviación en píxeles respecto al centro óptico, mostrándolo en un overlay.

## Estructura
| Carpeta | Contenido |
|---|---|
| `docs/` | Requisitos, diseño, registro de experimentos y actas |
| `libs/dpot` | Librería del ROA (DegorasOpticalTracking, GPL-3.0-or-later), copiada **sin modificar**; ver `libs/dpot/docs/` |
| `libs/frame_source` | Fuente de fotogramas: FITS, vídeo, simulador (sin Qt) |
| `libs/core` | Preprocesado, detección, centroide, haz, vector, estados (sin Qt) |
| `libs/astrometry` | Plate solving acotado (sin Qt) |
| `apps/gui` | Interfaz Qt Widgets |
| `apps/cli` | Procesado sin GUI, volcado a CSV |
| `prototypes/python` | Pruebas rápidas de filtros y algoritmos |
| `tests` | Pruebas y recortes pequeños de datos |

## Entorno
Se compila dentro del Dev Drive del ROA (MSYS2 + vcpkg, kits clang64 y ucrt64 de Qt Creator).
Clonar el repo en `<unidad>:\workspace`. Solo CMake.

## Datos
Los FITS y vídeos de observación **no** se suben al repositorio. Ubicación: _(por definir)_.

## Flujo de trabajo
Nada se sube directamente a `main`: rama propia → pull request → una aprobación → squash merge.

## Librería dpot, licencia y bases de estrellas
- `libs/dpot/` es la librería `dpot` de Ángel (ROA), copiada tal cual. Incluye faster-astap
  (MPL-2.0, con su `LICENSE` en `libs/dpot/thirdparty/faster-astap/`) y stb (MIT o dominio público).
- Como incorporamos código GPL-3.0-or-later, el repo lleva el texto de la GPL-3.0 en `LICENSE`.
- Las bases de estrellas de ASTAP (D80, 1,3 GB; G05 opcional), las imágenes de prueba y el índice
  de plate solving **no se suben**. Se descargan a la raíz del repo, en `databases/d80/` y
  `databases/g05/` (instrucciones en `docs/dpot_databases_LEEME.txt`). `databases/`, `images/` e `index/`
  están en `.gitignore`. Sin ellas los tests `IT_*` salen como Skipped.
