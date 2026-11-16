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