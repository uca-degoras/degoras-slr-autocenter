# 01 · Puesta en marcha

## 1. Entorno

El mismo que el del proyecto de bases de datos: el entorno D: con MSYS2 (`clang64` y `ucrt64`) y vcpkg. El proyecto no
usa ningún paquete de vcpkg: faster-astap y stb van dentro, en `thirdparty/`. Todo se compila desde el lanzador del
toolchain, que prepara PATH y las variables `DEVSYSTEM_*`:

- `D:\env\launcher\degoras_clang64_env_launcher.bat`
- `D:\env\launcher\degoras_ucrt64_env_launcher.bat`

El build queda en `$DEVSYSTEM_BUILDTREES/DegorasOpticalTracking_PoF/<toolchain>-dynamic-<debug|release>/`, con los
ejecutables en `bin/`. Si todavía no habéis quitado el triplete de vcpkg de `DEVSYSTEM_BUILDTREES` (ver el
`01_puesta_en_marcha` del proyecto de bases de datos), el árbol cuelga un nivel más abajo; funciona igual.

## 2. Datos

El proyecto busca sus datos en la carpeta que lo contiene (`DPOT_DATA_DIR`, por defecto `..`), que en el paquete es su
raíz:

| Carpeta | Contenido | Quién la llena |
|---|---|---|
| `images/` | Imágenes del RASA y de SFEL, y `frame_times.csv` | Viene en el paquete |
| `databases/d80/` | Base D80 de ASTAP (1476 ficheros `d80_*.1476`, 1,3 GB) | Vosotros, una vez |
| `databases/g05/` | Base G05 (290 ficheros `g05_*.290`, 102 MB), para campos de 3–20° | Opcional |
| `index/d80.idx` | Índice de cuadriláteros para resolver sin pista (0,9 GB) | Se crea solo la primera vez (~4 s) |

**Bases de estrellas.** Son de Han Kleijn (ASTAP), se publican en
<https://sourceforge.net/projects/astap-program/files/star_databases/>. Bajad las versiones que no se instalan:

- **D80** (necesaria): `d80_star_database.deb` (el `.zip` que enlaza la web de ASTAP ya no existe). Es un paquete Debian;
  se abre sin instalar nada. En el shell del lanzador:

  ```bash
  mkdir d80_tmp && cd d80_tmp
  ar x ../d80_star_database.deb       # deja data.tar.* entre otros
  tar -xf data.tar.*                  # los d80_*.1476 quedan en una carpeta de la instalación (opt/astap/)
  find . -name 'd80_*.1476' | head -3 # para ver dónde
  ```

  Copiad los `d80_*.1476` a `databases/d80/`. También vale 7-Zip: abrid el `.deb`, luego el `data.tar` de dentro.
- **G05** (opcional): `g05_star_database.zip`; descomprimidlo en `databases/g05/`.

No uséis los `.exe`: instalan en el sistema.

Sin la D80 todo compila y los tests unitarios pasan; los dos de integración salen como *Skipped*.

## 3. Compilar y pasar los tests

Desde `DegorasOpticalTracking_PoF/`, en el shell del lanzador:

```bash
cmake --workflow --preset dp-deb      # configura, compila y ejecuta todos los tests (Debug)
cmake --workflow --preset dp-rel      # lo mismo en Release
```

Criterio de "en verde": las cuatro combinaciones (`clang64`/`ucrt64` × `dp-deb`/`dp-rel`) compilan con **0 warnings** y
pasan el **100 % de los tests**. Son 15: los unitarios (`UT_*`, sin datos), los de faster-astap (`FA_*`), las reglas de
capas (`UT_LayerRules`) y dos de integración (`IT_SolveRasa`, `IT_SpotSfel`) sobre las imágenes reales. El primero que
se ejecute crea `index/d80.idx`.

## 4. Las herramientas

En `bin/` del build, desde el shell del lanzador. Las rutas son relativas a la raíz del paquete:

```bash
# Resolver las 17 imágenes de Matar sin pista, medir todas las FK5 y guardar las imágenes anotadas
dpot_identify --out out/matar databases/d80 DegorasOpticalTracking_PoF/data/catalogs 857 \
              images/RASA-8_ASI2600MMPRO/MATAR/*/*.PNG

# Lo mismo con pista (búsqueda en espiral desde la estrella objetivo, el método de ASTAP)
dpot_identify --hint databases/d80 DegorasOpticalTracking_PoF/data/catalogs 857 images/RASA-8_ASI2600MMPRO/MATAR/*/*.PNG

# La estrella de SFEL en una sesión: centroides, deriva, desvío respecto al eje, diafragma y exceso de verde
dpot_spot --out out/sfel images/SFEL-RX600_ASI585MCPRO/MATAR/*/*.PNG
```

Objetivos de las imágenes: ALTAIR = FK5 745, MATAR = FK5 857, POLARIS = FK5 907, FK51481 = FK5 1481. Las opciones están
al principio de cada `main.cpp` y en `03`.

## 5. Qt Creator

Lanzador `D:\env\launcher\degoras_<toolchain>_qtcreator.bat`; abrid `CMakeLists.txt` y elegid `qtcreator-dp-deb` o
`qtcreator-dp-rel`. Los ficheros se recogen con `GLOB`: uno nuevo aparece tras "Build" o "Run CMake".

## 6. Problemas típicos

| Síntoma | Causa y solución |
|---|---|
| `IT_*` salen "Skipped" | Falta `images/` o `databases/d80/` junto al proyecto. Bajad la D80 (sección 2) o cambiad `DPOT_DATA_DIR` |
| `dpot_identify`: "no capture time" | El fichero no está en una carpeta de sesión de ASICap ni su nombre empieza por la fecha. Ver `io/capture_time.h` |
| `CATALOG_NOT_FOUND` | La ruta de la D80 no tiene los `d80_*.1476` |
| La primera resolución tarda segundos | Se está creando `index/d80.idx`; las siguientes lo leen en milisegundos |
| Errores de rutas largas en `ucrt64` | Raíz de build demasiado larga: `CMakeUserPresets.json` desde el `.example` |
